use FileHandle;
use POSIX;
no warnings 'portable';
use strict;
use warnings;

if ($#ARGV < 1) {
	usage();
}

my $src_file = $ARGV[0];
my $dst_file = $ARGV[1];

open (my $infp, "<$src_file") or die $!;
open (my $outfp, ">$dst_file") or die $!;


my $line;
my %myhash1 = ();
my %myhash2 = ();
my %myhash3 = ();
my %myhash4 = ();
my $val1;
my $obj;
my $str;
my $total_pf = 0;
my $total_rf = 0;
my $total_rf_cross_gen = 0;

	while ($line = <$infp>)
	{
  		if ($line =~ m/(\w+)(:\s|\s)(\w+)/)
		{
			$str = $1;
			$obj = $3;
			
			if ($str =~ m/PF/)
			{
				$total_pf++;
  				$myhash1{$obj}++;
			}
			elsif ($str =~ m/RF/)
			{
				$total_rf++;
				$myhash2{$obj}++;
			}
			elsif ($str =~ m/Intra/)
			{
				$total_rf++;
				$total_rf_cross_gen++;
			}
  		}
	}


foreach $val1 (keys %myhash1)
{
	$myhash3{$myhash1{$val1}}++;
}

foreach $val1 (keys %myhash2)
{
	$myhash4{$myhash2{$val1}}++;
}

print $outfp "Total PF's -> $total_pf\n\n";
foreach $val1 (sort {$myhash3{$b} <=> $myhash3{$a}} keys %myhash3)
{
    print $outfp "[$val1] -> $myhash3{$val1}\n";
}

print $outfp "\nTotal RF's -> $total_rf Cross Gen -> $total_rf_cross_gen\n\n";
foreach $val1 (sort {$myhash4{$b} <=> $myhash4{$a}} keys %myhash4)
{
    print $outfp "[$val1] -> $myhash4{$val1}\n";
}

sub usage
{
	print "Usage: perl file2hash.pl <input_file> <output_file>\n";
	exit(1);
}
