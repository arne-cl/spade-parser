#!/local/bin/perl -w

$CHP = "your-path-to-Charniak's-parser-directory";
#$CHP = "/nfs/isd/radu/Work/Parsing/CharniakParser/";

use Cwd qw();
my $current_dir = Cwd::abs_path();
print "$current_dir\n";

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

        chdir($CHP) or die "cannot change to Charniak parser directory: $!\n";

	@args = ("./parse.sh $argv > $argv.chp");
	print STDERR "Charniak's syntactic parser in progress...\n";
	system(@args) == 0
	    or die "system @args failed: $?";
	print STDERR "Done.\n";

        chdir($current_dir) or die "cannot change to back to spade's bin directory: $!\n";

	@args = ("perl", "edubreak.pl", "$argv.chp");
	system(@args) == 0
	    or die "system @args failed: $?";
    }
    else{
	@args = ("./parse.sh $argv > $argv.chp");

        chdir($CHP) or die "cannot change to Charniak parser directory: $!\n";

	print STDERR "Charniak's syntactic parser in progress...\n";
	system(@args) == 0
	    or die "system @args failed: $?";
	print STDERR "Done.\n";

        chdir($current_dir) or die "cannot change to back to spade's bin directory: $!\n";

	@args = ("perl", "discparse.pl", "$argv.chp");
	system(@args) == 0
	    or die "system @args failed: $?";

	`rm -f $argv.chp.edu.fmt`;
	`rm -f $argv.chp.edu`;
    }
    `rm -f $argv.chp`;

}
