#!/usr/bin/perl

# Copyright (C) 2016 Martin Thomas <mtdev@hamtam.de>
# LGPL version 2 or later.

use strict;
use warnings;
use IO::Dir;
use File::Find qw(find);
use Encode::Guess;

my ($dir, $directory, $f, $w, $tmp);
my (@files, @dirs, $file, $filename);
my $header;
my $files;
my $encoding;
my @copyright;
my @copyrightout;
my @ctext;
my @uniqcpr;
my $i;

$encoding = ":encoding(UTF-8)";
$encoding = "";

$directory="./";

$header  = "Format: http://www.debian.org/doc/packaging-manuals/copyright-format/1.0/\n";
$header .= "Upstream-Name: uclibc-ng\n";
$header .= 'Upstream-Contact: Waldemar Brodkorb <wbx@uclibc-ng.org>'."\n";
$header .= "Source: git://uclibc-ng.org/git/uclibc-ng\n";

# my $emailregex='\b[[:alnum:]._%+-]+@[[:alnum:].-]+.[[:alpha:]]{2,6}\b';

sub list_dirs {
  my @dirs = @_;
  my @files;
  find({ wanted => sub { push @files, $_ } , no_chdir => 1 }, @dirs);
  return @files;
}

@files=list_dirs($directory);

foreach $file (@files) {
  if ( -f $file ){
#     $encoding = guess_encoding($file);
    open(my $fh, "< $encoding", $file)
      or die "Could not open file '$file' $!";
    while (my $row = <$fh>) {
      chomp $row;
      if ($row =~ m/[Cc]opyright / )
      {
        $row =~ s/^[\s\/\*#!;.\"\\]*//;
        $row =~ s/\s+$//;
        push @copyright, { file => $file, text => $row};
        last;
      }
    }
    close $fh
  }
}

@copyrightout = sort { $a->{text} cmp $b->{text} } @copyright;

$tmp="";
$i=0;
foreach (@copyrightout) {
  if ( $tmp eq $_->{'text'} )
  {
    print "       $_->{'file'}\n";
  }
  else
  {
    print "\n";
    print $header;
    print "Copyright: $_->{'text'}\n";
    print "License: GNU Lesser General Public License 2.1\n";
    print "Files: $_->{'file'}\n";
  }
  $tmp=$_->{'text'};
  ++$i;
}

