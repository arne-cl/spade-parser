#!/local/bin/perl -w

if( scalar(@ARGV)!= 1 && scalar(@ARGV)!= 3 ){
    print STDERR "Usage: discparse.pl parse-tree-file [training-file relations-file]\n" and exit;
}

$argv = shift;
@args = ("perl", "edubreak.pl", "$argv", "$argv.edu");
system(@args) == 0
    or die "system @args failed: $?";

@args = ("perl", "raw2fmt.pl", "$argv.edu");
system(@args) == 0
    or die "system @args failed: $?";

print STDERR "Sentence-level discourse parsing in progress...\n";
$dpd = `./dependencies $argv $argv.edu.fmt`;
if( $dpd eq "" ){
    print STDERR "Error (no ouput): dependencies $argv $argv.edu\n" and exit;
}

@slambda = ( 0.45, 0.45, 0.1 );  
@llambda = ( 0.9, 0.09, 0.009, 0.001 );

@smoothl = ( 0.6, 0.3, 0.09, 0.009, 0.001); # smoothing lambda
$smoothingES = 1;  # edu id smoothing on structures
$smoothingFS = 1;  # strong filter smoothing on structures 

$argv = shift;
if( $argv ){
    open(F, $argv) or die "Cannot open $argv";
}
else{ 
    open(F, "../resource/training.018")or die; 
}

$argv = shift;
if( $argv ){
    open(REL, $argv) or die "Cannot open $argv";
}
else{ 
    open(REL, "../resource/relations.018") or die; 
}

$flag = 0;
while( $line = <REL> ){
    chomp($line);
    $line =~ /([0-9]+) (.*)$/;
    $rel{$2} = $1;
    if( !$flag ){
	$maxrel = $2;
	$flag = 1;
    }
}

while( $line = <F> ){
    chomp($line);
    if( $line =~ /^([0-9]+) (.*?)$/ ){
	$line = $1;
    }
    if( $line>1 ){
	$line = <F>; 
	chomp($line);
	@input = split " ", $line;
	$n = scalar(@input)+1;
	$line = <F>; 
	chomp($line);
	if( $line ne "Struct:N/A" ){
	    @struct = split " ", $line;
	    for($i=0; $i<scalar(@struct); $i++){
		$struct = $struct[$i];
		@sdenval = sfilter(\@input, $struct, 0); 
		@ldenval = lfilter(\@input, $struct); 
		
		trainStructure($struct, \@sdenval);
		trainLabel($struct, \@ldenval);
	    }
	}
    }
    else{
	$line = <F>; 
	$line = <F>; 
    }
    $line = <F>; 
}

@dpd = split "\n", $dpd;
for($l=0; $l<scalar(@dpd); ){
    $line = $dpd[$l++];
    $line =~ /^([0-9]+) /;
    $n = $1;
    $text = $dpd[$l++];
    chomp($text);
    @leaf = split " \\| ", $text;
    $line = $dpd[$l++];
    if( $n>1 ){
	chomp($line);
	$input = $line;
	@input = split " ", $input;
	eduBoundaries();
	
	parse();
	label(1, $b[1][$n], $n);
	if( $p[1][$n]==0 ){ print STDERR "Cannot parse: $text\n"; }
	else{ 
	    print "(Root (span 1 $n)\n";
	    writeParse(1, $b[1][$n], $n, 2); print ")\n"; 
	}
    }
    elsif( $n==1 ){
	print "(Root (leaf 1)\n(text \_!$leaf[0]\_!)\n)\n";
    }
    else{
	print STDERR "No dependency info available for: $text\n";
    }
    
    $line = $dpd[$l++];
}
print STDERR "Done.\n";


sub parse{
    my ($i, $j, $s, $m, $struct, @sdenval, $snumval, $prob);

#initialize
    for($i=1; $i<=$n; $i++){
	for($j=1; $j<=$n; $j++){
	    $p[$i][$j] = 0;
	}
    }

#base
    for($i=1; $i<=$n; $i++){
	$p[$i][$i] = 1;
    }
    
    for($s=2; $s<=$n; $s++){
	for($i=1; $i<=$n-$s+1; $i++){
	    $j = $i+$s-1;
	    for($m=$i; $m<=$j-1; $m++){
		if( $p[$i][$m] && $p[$m+1][$j] ){
		    @prob = ( 0, 0, 0, 0, 0); # 0-NonSm, 1-ES, 2-FS, 3-EFS, 4-Default

		    $struct = "($i:$m,".($m+1).":$j)";

		    @sdenval = sfilter(\@input, $struct, 0);
		    $prob[0] = computeProb($struct, \@sdenval);
		    if( $smoothingES ){ # edu id smoothing
			$structsm = smoothing($struct); # creates @cinput
			@sdenval = sfilter(\@cinput, $structsm, 0);
			$prob[1] = computeProb($structsm, \@sdenval);
		    }
		    if( $smoothingFS ){ # strong filter smoothing	    
			@sdenval = sfilter(\@input, $struct, 1);
			$prob[2] = computeProb($struct, \@sdenval);
		    }
		    if( $smoothingES && $smoothingFS ){ 
			@sdenval = sfilter(\@cinput, $structsm, 1);
			$prob[3] = computeProb($structsm, \@sdenval);
		    }
		    $prob[4] = 10**(-6);
		    $prob = 0;
		    for($k=0; $k<5; $k++){
			$prob += $prob[$k]*$smoothl[$k];
		    }
						
		    if( $prob > $p[$i][$j] ){
			$p[$i][$j] = $prob;
			$b[$i][$j] = $m;
		    }
		}
	    }
	    if( $p[$i][$j]==$prob[4]*$smoothl[4] ){ #default value of m is now i - right branching
		if( $b[$i][$j-1] ){
		    $b[$i][$j] = $b[$i][$j-1]; #makes addition to existing right branching 
		}
	    }
	}
    }
    
}

sub computeProb{
    my ($struct, @sdenval, $prob, @prob, $k, @snumval);

    $struct = $_[0];
    @sdenval = @{$_[1]};

    for($k=0; $k<scalar(@sdenval); $k++){
	$snumval[$k] = "$struct | $sdenval[$k]";

	#$snum[$k]{$snumval[$k]} = 1; # makes joint prob?
	if( $sden[$k]{$sdenval[$k]} && $snum[$k]{$snumval[$k]} ){
	    $prob[$k] = $snum[$k]{$snumval[$k]}/$sden[$k]{$sdenval[$k]};
	}
	else{
	    $prob[$k] = 0;
	}
    }    
    
    for($k=0, $prob=0; $k<scalar(@sdenval); $k++){
	$prob += $slambda[$k]*$prob[$k];
    }
    return $prob;
}


sub label{
    my ($i, $m, $j, $k, $label1, $label2, $maxprob, $rel);

    $i = $_[0]; $m = $_[1]; $j = $_[2];

# default $maxrel is the most probable relation read from the file
    $maxprob = 0;
    foreach $rel (keys %rel){ 
	$struct = "($i:$m,".($m+1).":$j)";
	
	if( $rel eq "(Nucleus=Same-Unit,Nucleus=Same-Unit)" ){
	    if( $i==$m && $m+1==$j ){
		next;
	    }
	}

	@ldenval = lfilter(\@input, $struct);
	for($k=0; $k<scalar(@ldenval); $k++){
	    $lnumval[$k] = "$rel | $ldenval[$k]";

	    if( $lden[$k]{$ldenval[$k]} && $lnum[$k]{$lnumval[$k]} ){
		$prob[$k] = $lnum[$k]{$lnumval[$k]}/$lden[$k]{$ldenval[$k]};
	    }
	    else{
		$prob[$k] = 0;
	    }
	}
	for($k=0, $prob=0; $k<scalar(@ldenval); $k++){
	    $prob += $llambda[$k]*$prob[$k];
	}
	#$prob *= $rel{$rel};
	if( $prob > $maxprob ){
	    $maxrel = $rel;
	    $maxprob = $prob;
	}
    }
    $maxrel =~ /\((.*?),(.*?)\)/;
    $label1 = $1; $label2 = $2;
    
    $l[$i][$m] = $label1;
    $l[$m+1][$j] = $label2;
    if( $maxprob > 0 ){
	$p[$i][$j] *= $maxprob;    
    }

    if( $b[$i][$m] ){
	label($i, $b[$i][$m], $m);
    }
    if( $b[$m+1][$j] ){
	label($m+1, $b[$m+1][$j], $j);
    }

}


sub writeParse{
    my ($i, $m, $m1, $j, $offset, $label1, $status1, $label2, $status2, $label, $status);

    $i = $_[0]; $m = $_[1]; $j = $_[2]; $offset = $_[3];

    if( $i<$j ){
	$m1 = $m + 1;
	$label1 = $l[$i][$m]; $label2 = $l[$m1][$j];
	$label1 =~ /^(.*?)=(.*?)$/; $status1 = $1; $rel2par1 = $2;
	$label2 =~ /^(.*?)=(.*?)$/; $status2 = $1; $rel2par2 = $2;

	if( $m-$i > 0 ){
	    for(my $k=0; $k<$offset; $k++){ print " "; }
	    print "( $status1 (span $i $m) (rel2par $rel2par1)\n";
	    writeParse($i, $b[$i][$m], $m, $offset+2);
	    for(my $k=0; $k<$offset; $k++){ print " "; }
	    print " )\n";
	}
	else{
	    for(my $k=0; $k<$offset; $k++){ print " "; }
	    print "( $status1 (leaf $i) (rel2par $rel2par1)\n(text _!".$leaf[$i-1]."_!) )\n";
	}

	if( $j-$m1 > 0 ){
	    for(my $k=0; $k<$offset; $k++){ print " "; }
	    print "( $status2 (span $m1 $j) (rel2par $rel2par2)\n";
	    writeParse($m1, $b[$m1][$j], $j, $offset+2);
	    for(my $k=0; $k<$offset; $k++){ print " "; }
	    print " )\n";
	}
	else{
	    for(my $k=0; $k<$offset; $k++){ print " "; }
	    print "( $status2 (leaf $j) (rel2par $rel2par2)\n(text _!".$leaf[$j-1]."_!) )\n";
	}
    }
    else{
	$label = $l[$i][$j]; 
	$label =~ /^(.*?)=(.*?)$/; $status = $1; $rel2par = $2;

	for(my $k=0; $k<$offset; $k++){ print " "; }
	print "( $status (leaf $i) (rel2par $rel2par)\n(text _!".$leaf[$i-1]."_!) )\n";
    }

}

sub sfilter{
    my ($i, $k, @list, $struct, $v1, $v2, $v3, $v4, @denval, $flag);
 
    @list = @{$_[0]};
    $struct = $_[1];
    $flag = $_[2];

    if( $struct =~ /\((.*?)\:(.*?)\:(.*?),(.*?)\:(.*?)\:(.*?)\)/ ){
	$v1 = $1; $v2 = $3; $v3 = $4; $v4 = $6;
    }
    elsif( $struct =~ /\((.*?)\:(.*?),(.*?)\:(.*?)\)/ ){
	$v1 = $1; $v2 = $2; $v3 = $3; $v4 = $4;
    }
    %fs1 = (); %fs2 = ();
    #$fs1{$v1} = 1; $fs1{$v2} = 1; #considers only the given edus
    #$fs2{$v3} = 1; $fs2{$v4} = 1; 
    for($k=$v1; $k<=$v2; $k++){ $fs1{$k} = 1; } #considers interval between edus
    for($k=$v3; $k<=$v4; $k++){ $fs2{$k} = 1; }

    @denval = ( "", "", "" );
    if( $flag==0 ){
	@denval = weaksfilter(\@list); #strong filter is too strong for structures
    }
    else{ 
	@denval = strongsfilter(\@list); # strong filter as smoothing
    }
    return @denval;
}

sub lfilter{
    my ($i, $k, @list, $struct, $v1, $v2, $v3, $v4, @denval);
 
    @list = @{$_[0]};
    $struct = $_[1];

    if( $struct =~ /\((.*?)\:(.*?)\:(.*?),(.*?)\:(.*?)\:(.*?)\)/ ){
	$v1 = $1; $v2 = $3; $v3 = $4; $v4 = $6;
    }
    elsif( $struct =~ /\((.*?)\:(.*?),(.*?)\:(.*?)\)/ ){
	$v1 = $1; $v2 = $2; $v3 = $3; $v4 = $4;
    }
    %fs1 = (); %fs2 = ();
    #$fs1{$v1} = 1; $fs1{$v2} = 1; #considers only the given edus
    #$fs2{$v3} = 1; $fs2{$v4} = 1; 
    for($k=$v1; $k<=$v2; $k++){ $fs1{$k} = 1; } #considers interval between edus
    for($k=$v3; $k<=$v4; $k++){ $fs2{$k} = 1; }

    @denval = stronglfilter(\@list); 
    if( $denval[0] eq "" ){
	@denval = weaklfilter(\@list, @denval);
    }

    return @denval;
}

sub strongsfilter{
    my (@list, $i, $k, $v1, $v2, $v3, $v4, $v5, $v6, $v7, @val, @denval);

    @list = @{$_[0]};
    @denval = ( "", "", "");

    for($i=0; $i<scalar(@list); $i++){
	$list[$i] =~ /(.*?)\[(.*?)\/(.*?)\]<\[(.*?)\](.*?)\[(.*?)\/(.*?)\]/;
	$v1 = $1; $v2 = $2; $v3 = $3; $v4 = $4; $v5 = $5; $v6 = $6; $v7 = $7;  

	$val[0] = "$v1\[$v3\]<$v5 ";
	$val[1] = "$v1<$v5\[$v7\] ";
	$val[2] = "$v1<$v5 ";


	if( ($fs1{$v1} && $fs2{$v5} ) || ($fs1{$v5} && $fs2{$v1}) ){
	    for($k=0; $k<scalar(@denval); $k++){
		if( $denval[$k] !~ /$v1(.*?)<$v5(.*?)/ && $v1!=$v5 ){ 
		    $denval[$k] .= $val[$k];
		}
	    }
	}
    }

    for($k=0; $k<scalar(@denval); $k++){
	chop($denval[$k]);
    }

    return @denval;
}

sub weaksfilter{
    my (@list, $i, $k, $v1, $v2, $v3, $v4, $v5, $v6, $v7, @val, @denval);

    @list = @{$_[0]};
    @denval = ( "", "", "");

    for($i=0; $i<scalar(@list); $i++){
	$list[$i] =~ /(.*?)\[(.*?)\/(.*?)\]<\[(.*?)\](.*?)\[(.*?)\/(.*?)\]/;
	$v1 = $1; $v2 = $2; $v3 = $3; $v4 = $4; $v5 = $5; $v6 = $6; $v7 = $7;  

	$val[0] = "$v1\[$v3\]<$v5 ";
	$val[1] = "$v1<$v5\[$v7\] ";
	$val[2] = "$v1<$v5 ";


	if( ($fs1{$v1} || $fs2{$v5} ) || ($fs1{$v5} || $fs2{$v1}) ){
	    for($k=0; $k<scalar(@denval); $k++){
		if( $denval[$k] !~ /$v1(.*?)<$v5(.*?)/  && $v1!=$v5 ){ 
		    $denval[$k] .= $val[$k];
		}
	    }
	}
    }

    for($k=0; $k<scalar(@denval); $k++){
	chop($denval[$k]);
    }

    return @denval;
}

sub stronglfilter{
    my (@list, $i, $k, $v1, $v2, $v3, $v4, $v5, $v6, $v7, @lval, @denval);

    @list = @{$_[0]};
    @denval = ("", "", "", "");

    for($i=0; $i<scalar(@list); $i++){
	$list[$i] =~ /(.*?)\[(.*?)\/(.*?)\]<\[(.*?)\](.*?)\[(.*?)\/(.*?)\]/;
	$v1 = $1; $v2 = $2; $v3 = $3; $v4 = $4; $v5 = $5; $v6 = $6; $v7 = $7;  
	$v4 = "N/A"; # remove the parent label

	if( $v1<$v5 ){
	    $lval[0] = "\[$v2/$v3\]<$v4\[$v6/$v7\] ";
	    $lval[1] = "\[$v2/$v3\]<$v4\[$v7\] ";
	    $lval[2] = "\[$v3\]<$v4\[$v6/$v7\] ";
	    $lval[3] = "\[$v3\]<$v4\[$v7\] ";
	}
	else{
	    $lval[0] = "\[$v6/$v7\]>$v4\[$v2/$v3\] ";
	    $lval[1] = "\[$v7\]>$v4\[$v2/$v3\] ";
	    $lval[2] = "\[$v6/$v7\]>$v4\[$v3\] ";
	    $lval[3] = "\[$v7\]>$v4\[$v3\] ";
	}

	if( ($fs1{$v1} && $fs2{$v5} ) || ($fs1{$v5} && $fs2{$v1}) ){
	    for($k=0; $k<scalar(@denval); $k++){
		if($denval[$k] !~ /\[(.*?)$v3\]<$v4\[(.*?)$v7\]/ &&
		   $denval[$k] !~ /\[(.*?)$v7\]>$v4\[(.*?)$v3\]/ ){ 
		    $denval[$k] .= $lval[$k];
		}
	    }
	}
    }

    for($k=0; $k<scalar(@denval); $k++){
	chop($denval[$k]);
    }

    return @denval;
}

sub weaklfilter{
    my (@list, $i, $k, $v1, $v2, $v3, $v4, $v5, $v6, $v7, @lval, @denval);

    @list = @{$_[0]};
    @denval = ("", "", "", "");

    for($i=0; $i<scalar(@list); $i++){
	$list[$i] =~ /(.*?)\[(.*?)\/(.*?)\]<\[(.*?)\](.*?)\[(.*?)\/(.*?)\]/;
	$v1 = $1; $v2 = $2; $v3 = $3; $v4 = $4; $v5 = $5; $v6 = $6; $v7 = $7;  
	$v4 = "N/A"; # remove the parent label

	if( $v1<$v5 ){
	    $lval[0] = "\[$v2/$v3\]<$v4\[$v6/$v7\] ";
	    $lval[1] = "\[$v2/$v3\]<$v4\[$v7\] ";
	    $lval[2] = "\[$v3\]<$v4\[$v6/$v7\] ";
	    $lval[3] = "\[$v3\]<$v4\[$v7\] ";
	}
	else{
	    $lval[0] = "\[$v6/$v7\]>$v4\[$v2/$v3\] ";
	    $lval[1] = "\[$v7\]>$v4\[$v2/$v3\] ";
	    $lval[2] = "\[$v6/$v7\]>$v4\[$v3\] ";
	    $lval[3] = "\[$v7\]>$v4\[$v3\] ";
	}

	if( ($fs1{$v1} || $fs2{$v5} ) || ($fs1{$v5} || $fs2{$v1}) ){
	    for($k=0; $k<scalar(@denval); $k++){
		if($denval[$k] !~ /\[(.*?)$v3\]<$v4\[(.*?)$v7\]/ &&
		   $denval[$k] !~ /\[(.*?)$v7\]>$v4\[(.*?)$v3\]/ ){ 
		    $denval[$k] .= $lval[$k];
		}
	    }
	}
    }

    for($k=0; $k<scalar(@denval); $k++){
	chop($denval[$k]);
    }

    return @denval;
}

sub trainStructure{
    my ($struct, $cstruct, $k, @numval, @denval, $v1, $v2, $v3, $v4);

    $struct = $_[0];
    @denval = @{$_[1]};

    $struct =~ /\((.*?)\:(.*?)\:(.*?),(.*?)\:(.*?)\:(.*?)\)/;
    $v1 = $1; $v2 = $3; $v3 = $4; $v4 = $6;

    for($k=0; $k<scalar(@denval); $k++){
	$numval[$k] = "($v1:$v2,$v3:$v4) | $denval[$k]";
	
	if( $snum[$k]{$numval[$k]} ){ $snum[$k]{$numval[$k]} += $smoothl[0]; }
	else{ $snum[$k]{$numval[$k]} = $smoothl[0]; }
	if( $sden[$k]{$denval[$k]} ){ $sden[$k]{$denval[$k]} += $smoothl[0]; }
	else{ $sden[$k]{$denval[$k]} = $smoothl[0]; }
    }

    if( $smoothingES ){
	$cstruct = smoothing($struct);
	@denval = sfilter(\@cinput, $cstruct, 0);
	$struct =~ /\((.*?)\:(.*?)\:(.*?),(.*?)\:(.*?)\:(.*?)\)/;
	$v1 = $1; $v2 = $3; $v3 = $4; $v4 = $6;

	for($k=0; $k<scalar(@denval); $k++){
	    $numval[$k] = "($v1:$v2,$v3:$v4) | $denval[$k]";
	    
	    if( $snum[$k]{$numval[$k]} ){ $snum[$k]{$numval[$k]} += $smoothl[1]; }
	    else{ $snum[$k]{$numval[$k]} = $smoothl[1]; }
	    if( $sden[$k]{$denval[$k]} ){ $sden[$k]{$denval[$k]} += $smoothl[1]; }
	    else{ $sden[$k]{$denval[$k]} = $smoothl[1]; }
	}
    }
    
    if( $smoothingFS ){
	@denval = sfilter(\@input, $struct, 1);
	$struct =~ /\((.*?)\:(.*?)\:(.*?),(.*?)\:(.*?)\:(.*?)\)/;
	$v1 = $1; $v2 = $3; $v3 = $4; $v4 = $6;

	for($k=0; $k<scalar(@denval); $k++){
	    $numval[$k] = "($v1:$v2,$v3:$v4) | $denval[$k]";
	    
	    if( $snum[$k]{$numval[$k]} ){ $snum[$k]{$numval[$k]} += $smoothl[2]; }
	    else{ $snum[$k]{$numval[$k]} = $smoothl[2]; }
	    if( $sden[$k]{$denval[$k]} ){ $sden[$k]{$denval[$k]} += $smoothl[2]; }
	    else{ $sden[$k]{$denval[$k]} = $smoothl[2]; }
	}
    }
    
    if( $smoothingES && $smoothingFS ){
	$cstruct = smoothing($struct);
	@denval = sfilter(\@cinput, $cstruct, 1);
	$struct =~ /\((.*?)\:(.*?)\:(.*?),(.*?)\:(.*?)\:(.*?)\)/;
	$v1 = $1; $v2 = $3; $v3 = $4; $v4 = $6;

	for($k=0; $k<scalar(@denval); $k++){
	    $numval[$k] = "($v1:$v2,$v3:$v4) | $denval[$k]";
	    
	    if( $snum[$k]{$numval[$k]} ){ $snum[$k]{$numval[$k]} += $smoothl[3]+$smoothl[4];}
	    else{ $snum[$k]{$numval[$k]} = $smoothl[3]+$smoothl[4]; }
	    if( $sden[$k]{$denval[$k]} ){ $sden[$k]{$denval[$k]} += $smoothl[3]+$smoothl[4]; }
	    else{ $sden[$k]{$denval[$k]} = $smoothl[3]+$smoothl[4]; }
	}
    }
    return;
}

sub trainLabel{
    my ($struct, $k, @numval, @denval, $v1, $v2);

    $struct = $_[0];
    @denval = @{$_[1]};

    $struct =~ /\((.*?)\:(.*?)\:(.*?),(.*?)\:(.*?)\:(.*?)\)/;
    $v1 = $2; $v2 = $5; 

    for($k=0; $k<scalar(@denval); $k++){
	$numval[$k] = "($v1,$v2) | $denval[$k]";
  
	if( $lnum[$k]{$numval[$k]} ){ $lnum[$k]{$numval[$k]} += 1; }
	else{ $lnum[$k]{$numval[$k]} = 1; }
	if( $lden[$k]{$denval[$k]} ){ $lden[$k]{$denval[$k]} += 1; }
	else{ $lden[$k]{$denval[$k]} = 1; }
    }

}

sub mapping1{
    my ($struct, $n, $i, $v1, $v2, $v3, $v4, $d);
    
    $struct = $_[0];
    $n = $_[1];

    if( $struct =~ /\((.*?)\:(.*?)\:(.*?),(.*?)\:(.*?)\:(.*?)\)/ ){
	$v1 = $1; $v2 = $3; $v3 = $4; $v4 = $6;
    }
    elsif( $struct =~ /\((.*?)\:(.*?),(.*?)\:(.*?)\)/ ){
	$v1 = $1; $v2 = $2; $v3 = $3; $v4 = $4;
    }

    # shifting towards the populated space
    $d = $v1 - 1;
    for($i=1; $i<=$n; $i++){
	if( $d >= 3 ){
	    $map[$i] = $i - $d + 1; 
	}
    }
}

sub mapping2{
    my ($struct, $n, $i, $v1, $v2, $v3, $v4, $d1, $d2);
    
    $struct = $_[0];
    $n = $_[1];

    if( $struct =~ /\((.*?)\:(.*?)\:(.*?),(.*?)\:(.*?)\:(.*?)\)/ ){
	$v1 = $1; $v2 = $3; $v3 = $4; $v4 = $6;
    }
    elsif( $struct =~ /\((.*?)\:(.*?),(.*?)\:(.*?)\)/ ){
	$v1 = $1; $v2 = $2; $v3 = $3; $v4 = $4;
    }

    # making long units to be of length 2
    $d1 = $v2-$v1;
    if( $d1>1 ){ 
	for($i=$v1+1; $i<=$n; $i++){ 
	    if( $i-$d1+1 > $v1 ){
		$map[$i] = $i-$d1+1; 
	    }
	    else{ $map[$i] = $v1; }
	} 
    }
    $d2 = $v4-$v3;
    if( $d2>1 ){ 
	for($i=$v3+1; $i<=$n; $i++){ 
	    if( $i-$d2+1 > $v3 ){
		$map[$i] = $i-$d2+1; 
	    }
	    else{ $map[$i] = $v3; }
	} 
    }
		 
}

sub smoothing{
    my ( $k, $v1, $v5, $struct );

    $struct = $_[0];
    for($k=1; $k<=$n; $k++){ $map[$k] = $k; }
    
    mapping1($struct, $n);
    #mapping2($struct, $n); # does not work
    if( $struct =~ /\((.*?):(.*?):(.*?),(.*?):(.*?):(.*?)\)/ ){
	$struct = "($map[$1]:$map[$3],$map[$4]:$map[$6])";
    }
    elsif( $struct =~ /\((.*?):(.*?),(.*?):(.*?)\)/ ){
	$struct = "($map[$1]:$map[$2],$map[$3]:$map[$4])";
    }

    @cinput = @input; 
    for($k=0; $k<scalar(@input); $k++){
	$cinput[$k] =~ /(.*?)\[(.*?)\/(.*?)\]<\[(.*?)\](.*?)\[(.*?)\/(.*?)\]/;
	$v1 = $1;  $v5 = $5; 
	if( $v1>0 ){ $cinput[$k] =~ s/$v1\[/$map[$v1]\[/; } 
	if( $v5>0 ){ $cinput[$k] =~ s/\]$v5\[/\]$map[$v5]\[/; } 
    }

    return $struct;
}

sub eduBoundaries{
    my ( $k );

    %eduB = (); %eduE = (); 

    for($k=0; $k<scalar(@input); $k++){
	$input[$k] =~ /(.*?)\{(.*?):(.*?)\}\[(.*?)\/(.*?)\]<\[(.*?)\](.*?)\{(.*?):(.*?)\}\[(.*?)\/(.*?)\]/; 
	$input[$k] = "$1\[$4/$5\]<\[$6\]$7\[$10/$11\]";
	$eduB{$1} = $2; $eduE{$1} = $3;
	$eduB{$7} = $8; $eduE{$7} = $9;
    }
}


