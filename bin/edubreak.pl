#!/local/bin/perl -w

if( scalar(@ARGV)!= 1 && scalar(@ARGV)!= 2 ){
    print STDERR "Usage: edubreak.pl parse-tree-file [output-file]\n" and exit;
}

$outfile = 0;
$output = `./edubreak $ARGV[0]`;
if( $output eq "" ){
    print STDERR "Error (no ouput): edubreak $ARGV[0]\n" and exit;
}
else{
    print STDERR "Discourse segmentation in progress...\n";
    if( $ARGV[1] ){
	open(FO, ">$ARGV[1]") or die "Cannot open for write $ARGV[1]";
	$outfile = 1;
    }
    ruleBasedFix($output);
    print STDERR "Done.\n";
}

sub ruleBasedFix
{
    my ($output) = @_;

    @output = split "\n", $output;
    $n = 0; @line = ();
    for($i=0; $i<scalar(@output); $i++){
	$line = $output[$i];
	if( $line ne "" ){
	    $line =~ s/ ` ` / `` /g; $line =~ s/^` ` /`` /g;
	    $line =~ s/ ' ' / '' /g; $line =~ s/ ' '$/ ''/g; $line =~ s/^' '/''/g;
	}
	$line[$n++] = $line;
    }
    for($k=0; $k<$n;){
	if( $line[$k] eq "" ){
	    for($kk=$k; $kk<$n-1; $kk++){
		$line[$kk] = $line[$kk+1];
	    }
	    $n -= 1;
	}
	else{
	    $k++;
	}
    }
    for($k=0; $k<$n; $k++){
	$skip = 0;
	if( $k+1<$n ){
	    if( $line[$k+1] =~ /(^\:( *)$|^\<S\>( *)$|^\. <S>( *)$|^\;( *)$|^,( *)$|^, ''( *)$)/ ){
		$line[$k] .= $line[$k+1];
		$skip = 1;
	    }
	    elsif( $line[$k+1] =~ /(^, |^' )(.*)/ ){
	         $line[$k] .= $1;
	         $line[$k+1] = $2;
                 $line[$k] =~ s/ ' ' $/ ''/;
	         $skip = 0;
	    }       
            elsif( $line[$k] !~ /<S>$/ && $line[$k+1] =~ /^(.+? )(-- .+?)$/){
                 $line[$k] .= $1; 
	         $line[$k+1] = $2;
	         $skip = 0;
            }
            elsif( $line[$k] =~ /-- / && $line[$k] !~ /<S>$/ && $line[$k+1] =~ /^(-- )(.+)/ ){
                 $line[$k] .= $1; 
	         $line[$k+1] = $2;
	         $skip = 0;
	    }
            elsif( $line[$k] !~ /<S>$/ && ($line[$k+1] =~ /^--( *)$/ || $line[$k+1] =~ /^-- <S>$/)){
                 $line[$k] .= $line[$k+1];
        	 $skip = 1;
	    }
            elsif( $line[$k] =~ /-- / && $line[$k+1] =~ /^--( *)$/ ){
                 $line[$k] .= $line[$k+1];
        	 $skip = 1;
	    }
            elsif( $line[$k] =~ /^--( *)$/ ){
                 $line[$k+1] = $line[$k].$line[$k+1];
                 $k += 1; # skip now
	         $skip = 0;
	    }       
	    else{ $skip = 0; }	    
         if( $outfile ){
           print FO $line[$k]. "\n";
         }
         else{
           print $line[$k]. "\n";
         }
	 $k += $skip;
	}
        else{ 
          if( $outfile ){
            print FO $line[$k]. "\n";
          } 
          else{
            print $line[$k]. "\n";
          }
        }
    }
}
