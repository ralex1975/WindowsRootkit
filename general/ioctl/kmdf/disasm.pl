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
open (my $out, ">$dst_file") or die $!;


my $line;
my $nextline;
my $pc;
my $pc_dec;
my @iret_arr;
my $iret_arr_idx = 0;
my @intr_arr;
my $intr_arr_idx = 0;
my @sysret_arr;
my $sysret_arr_idx = 0;

	while ($line = <$infp>)
	{
  		if ($line =~ m/(\w+):.*swapgs/)
		{
			$pc = $1;
			$nextline = <$infp>;
			if ($nextline =~ m/iret/)
			{
				$pc_dec = hex($pc) - 37;
				$pc = sprintf("0x%XULL", $pc_dec);
				$iret_arr[$iret_arr_idx] = $pc;
				$iret_arr_idx++;
				#print "iret:$pc\n";
			}
			elsif ($nextline =~ m/gs:/)
			{
				$pc_dec = hex($pc);
				$pc = sprintf("0x%XULL", $pc_dec);
				$intr_arr[$intr_arr_idx] = $pc;
				$intr_arr_idx++;
				#print "intr:$pc\n";
			}
			elsif ($nextline =~ m/sysret/)
			{
				$pc_dec = hex($pc) - 6;
				$pc = sprintf("0x%XULL", $pc_dec);
				$sysret_arr[$sysret_arr_idx] = $pc;
				$sysret_arr_idx++;
			}
  		}
	}

	my $i;
	if ($iret_arr_idx)
	{
		print $out "unsigned long long iret_funcs[] = {\n";
		for ($i = 0; $i < $iret_arr_idx-1; $i++)
		{
			print $out "\t$iret_arr[$i],\n";
		}
		print $out "\t$iret_arr[$i]\n";
		print $out "};\n\n";
	}


	if ($intr_arr_idx)
	{
		print $out "unsigned long long intr_funcs[] = {\n";
		for ($i = 0; $i < $intr_arr_idx-1; $i++)
		{
			print $out "\t$intr_arr[$i],\n";
		}
		print $out "\t$intr_arr[$i]\n";
		print $out "};\n\n";
	}

	if ($sysret_arr_idx)
	{
		print $out "unsigned long long sysret_funcs[] = {\n";
		for ($i = 0; $i < $sysret_arr_idx-1; $i++)
		{
			print $out "\t$sysret_arr[$i],\n";
		}
		print $out "\t$sysret_arr[$i]\n";
		print $out "};\n";
	}


sub usage
{
	print "Usage: perl file2hash.pl <input_file> <output_file>\n";
	exit(1);
}
