#! /usr/bin/env perl -w

use strict;

use Getopt::Long;

sub class2file;

my $opt_parent = 'GObject';

my $opt_class = shift @ARGV;
die "usage" unless defined $opt_class && length $opt_class;

my $file_stem = class2file $opt_class;
my $macro_stem = $file_stem;
$macro_stem =~ tr/-/_/g;

print <<EOF;
$opt_class
$file_stem
$macro_stem
EOF

sub class2file {
	my ($name) = @_;

	$name =~ s/[A-Z][a-z]+/$&-/g;
	chop $name;
	$name =~ s/[A-Z]+(?=[A-Z])/$&-/g;

	return lc $name;
}
