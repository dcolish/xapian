#! /usr/bin/perl -w

# mkdoc.pl: generate documentation from source code and associated files.
use strict;

# Declarations
sub get_descriptions();
sub tohtml($);
sub output_html();

# Parse command line parameters
if ($#ARGV < 2 || $#ARGV > 3) {
  print "usage: mkdoc.pl <version> <source directory> <destination> [webroot]\n";
  exit 1;
}

my ($version, $srcdir, $dest, $webroot) = @ARGV;
$webroot = "" unless defined $webroot;

my %descriptions = ();
my %classes = ();

get_descriptions();
#get_codestruct();
output_html();

sub get_descriptions() {
    # Assume we have find.  Get all the possible directories.
    my $subdirs;
    open M, "$srcdir/Makefile" or die $!;
    while (<M>) {
	while (s/\\\n$//) { $_ .= <M>; }
	if (s/^\s*DIST_SUBDIRS\s*=\s*//) {
	    s/\s*$//;
	    $subdirs = join " ", map {"$srcdir/$_"} split /\s+/;
	    last;
        }
    }
    close M;
    die "DIST_SUBDIRS not found in Makefile" unless defined $subdirs;

    my @dirs = split(/\n/, `find $subdirs -type d`);

    # Read the contents of any dir_contents's we find.
    my $dir;
    foreach $dir (@dirs) {
	next if $dir =~ m/CVS$/;
	my $contentsfile = "$dir/dir_contents";
	next if ! -r $contentsfile;
	open(CONTENTSFILE, $contentsfile);

	my $contents = "";
	while(<CONTENTSFILE>) { $contents .= $_; }

        # Get directory tag
	if($contents !~ m#<directory>\s*(.+?)\s*</directory>#is) {
	    print STDERR "Skipping $contentsfile: didn't contain a directory tag\n";
	    next;
	}
	my $directory = $1;
	my $tagdir = "$srcdir/$directory/";
	if($directory eq "ROOT") {
            # Special case for top level dir
	    $tagdir = "$srcdir/";
	}
	$dir = "$dir/";
	$dir =~ s!/(?:\./)+!/!g;
	$tagdir =~ s!/(?:\./)+!/!g;
	if("$tagdir" ne $dir) {
	    print STDERR "Skipping $contentsfile: incorrect directory tag\n";
	    print STDERR "`$tagdir' != `$dir'\n";
	    next;
	}

        # Get description tag
	if($contents !~ m#<description>\s*(.+?)\s*</description>#is) {
	    print STDERR "Skipping $contentsfile: didn't contain a description tag\n";
	    next;
	}
	$descriptions{$directory} = "$1";
    }
    close(CONTENTSFILE);
}

sub get_codestruct() {
    my @files = split(/\n/, `find $srcdir -name \*.cc -o -name \*.c -o -name \*.cpp -o -name \*.h`);

    my $file;
    foreach $file (@files) {
	open(CODEFILE, $file);
	print "$file\n";

	my $contents;
	while(<CODEFILE>) {
	    $contents .= $_;
	}

	my $commentno = 1;
	while($contents =~ m#(/\*.*?\*/)#s) {
	    print "comment: `$1'\n";
	    my $newlines = "";

	    $contents = "$`/*$newlines*/$'";
	}
	while($contents =~ s#//(.*?)\n#\n#) { print "removing $1\n"}
	while($contents) {
	    $contents =~ /\bclass\s+(\w+)\s*(?:|:\s*(.+?)\s*)/;
	    my $classname = $1;
	    my $inheritance = "";
	    $inheritance = $2 if defined $2;
	    print "$classname-$inheritance\n";
	}
    }
    close(CODEFILE);
}

sub tohtml($) {
  my $html = $_[0];
  $html =~ s#&#&amp;#g;
  $html =~ s#>#&gt;#g;
  $html =~ s#<#&lt;#g;
  $html =~ s#"#&quot;#g;
  $html =~ s#\n\n#\n<P>\n#g;
  return $html;
}

sub output_html() {
    # Open output
    open(DESTFILE, ">$dest");

    # Print header
    print DESTFILE <<EOF;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML>
<HEAD>
<TITLE>Xapian: Code structure</TITLE>
</HEAD>
<BODY BGCOLOR="white">
This documentation was automatically generated, and corresponds to version
$version of Xapian.
<HR>
EOF

    # Print directory index
    print DESTFILE "<H1>Directory structure index</H1>\n";
    my $level=0;
    my $dir;
    foreach $dir (sort keys(%descriptions)) {
	my $newlev = ($dir =~ tr#/##) + 1; # Count the number of /'s in $dir
	if($level == $newlev) {
	    print DESTFILE "</LI>";
	}
	while($level < $newlev) {
	    print DESTFILE "\n" . " " x ($level * 2) . "<UL>";
	    $level++;
	}
	while($level > $newlev) {
	    print DESTFILE "</LI>\n" . " " x ($level * 2 - 2) . "</UL>";
	    $level--;
	}
	print DESTFILE "\n" . " " x ($level * 2 - 1) .
	"<LI><A HREF=\"#$dir\">$dir</A>";
    }
    my $newlev=0;
    while($level > $newlev) {
	print DESTFILE "</LI>\n" . " " x ($level * 2 - 2) . "</UL>";
	$level--;
    }
    print DESTFILE "\n<HR>\n\n";

    # Print directory details
    print DESTFILE "<H1>Directory structure</H1>\n";
    foreach $dir (sort keys(%descriptions)) {
	print DESTFILE "<A NAME=\"$dir\"></A>";
	if($webroot) {
	    if($dir eq "ROOT") {
		print DESTFILE "<A HREF=\"$webroot\">";
	    } else {
		print DESTFILE "<A HREF=\"$webroot/$dir\">";
	    }
	    print DESTFILE "<H2>$dir</H2></A>\n\n";
	} else {
	    print DESTFILE "<H2>$dir</H2>\n\n";
	}
	print DESTFILE &tohtml($descriptions{$dir});
	print DESTFILE "\n\n\n";
    }

    # Print footer
    my $date = `date "+%d %B %Y"`;
    chomp $date;

    print DESTFILE <<EOF;
<HR>
Generated on $date.
<P>
Command line used to generate this documentation:<BR>
<CODE>$0 @ARGV</CODE>
</BODY>
</HTML>
EOF

    # Done
    close DESTFILE;
}
