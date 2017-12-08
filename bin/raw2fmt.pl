#!/local/bin/perl -w

for($i=0; $i<scalar(@ARGV); $i++){
    open(F, $ARGV[$i]) or die;
    $name = $ARGV[$i];
    open(FO, ">$name.fmt") or die "Cannot open for write $name.fmt";

    while( 1 ){
	$cnt = 0;
	while( $line=<F> ){
	    chomp($line);
	    $store[$cnt] = $line;
	    @line = split " ", $line;
	    $nstore[$cnt] = scalar(@line);
	    if( $line[scalar(@line)-1] eq "<S>" ){
		$nstore[$cnt] -= 1;
		$store[$cnt] =~ s/<S>//g;
		$cnt++;
		last;
	    }
	    $cnt++;
	}
	if( $cnt==0 ){
	    last;
	}
	print FO "$cnt\n";
	for($k=0; $k<$cnt; $k++){
	    print FO "$nstore[$k] $store[$k]\n";
	}
    }
    close(F);
    close(FO);
    #printf STDERR "Output written on $name.fmt\n";
}
