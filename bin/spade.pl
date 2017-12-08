#!/local/bin/perl -w

$CHP = "your-path-to-Charniak's-parser-directory";
#$CHP = "/nfs/isd/radu/Work/Parsing/CharniakParser/";

if( scalar(@ARGV)!= 1 && scalar(@ARGV)!= 2 ){
    print STDERR "Usage: spade.pl [-seg-only] one-sent-per-line-file\n";
}
else{
    print STDERR "\n";
    print STDERR "+--------------------------------------------------+\n";
    print STDERR "|         SPADE, Release Developer 0.9             |\n";
    print STDERR "|    (c) 2003 University of Southern California    |\n";
    print STDERR "|         written by Radu Soricut                  |\n";
    print STDERR "+--------------------------------------------------+\n";
    print STDERR "\n";

    if( $CHP eq "your-path-to-Charniak's-parser-directory" ){
	print STDERR "You need to set the path to Charniak's parser directory first.\n" and exit;
    }

    $argv = shift;
    if( $argv eq "-seg-only" ){
	$argv = shift;
	@args = ("$CHP/parseIt $CHP/DATA/ $argv > $argv.chp");
	print STDERR "Charniak's syntactic parser in progress...\n";
	system(@args) == 0
	    or die "system @args failed: $?";
	print STDERR "Done.\n";

	@args = ("perl", "edubreak.pl", "$argv.chp");
	system(@args) == 0
	    or die "system @args failed: $?";
    }
    else{
	@args = ("$CHP/parseIt $CHP/DATA/ $argv > $argv.chp");
	print STDERR "Charniak's syntactic parser in progress...\n";
	system(@args) == 0
	    or die "system @args failed: $?";
	print STDERR "Done.\n";

	@args = ("perl", "discparse.pl", "$argv.chp");
	system(@args) == 0
	    or die "system @args failed: $?";

	`rm -f $argv.chp.edu.fmt`;
	`rm -f $argv.chp.edu`;
    }
    `rm -f $argv.chp`;

}
