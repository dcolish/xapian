#! /usr/bin/perl -w
# todo2html.pl: generate documentation from source code and associated files.
require 5;
use strict;
use XML::Parser;
use Text::Format;
# FIXME: HTML escape output

my %priority = qw(
U Urgent
H High
M Medium
L Low
V Verylow
D Dubious
);

my %priorityorder = qw(
U 6
H 5
M 4
L 3
V 2
D 1
);

my %priority_colours = qw(
U ff7777
H 77ff77
M 77dd77
L 77bb77
V 779977
D 777777
);

$priority{' '} = '';
$priorityorder{' '} = 0;

my %field;

my ($preamble, $postamble, $table_begin, $table_end);

my $text = 0;
my $filter_release = "";
if ($ARGV[0] eq '--release') {
   shift @ARGV;
   $filter_release = shift @ARGV;
}

if ($ARGV[0] eq '--text') {
   shift @ARGV;
   $text = 1;
   $preamble =
"This file contains items which need to be worked on.

[P]riority values, in decreasing priority order, are:

 [U]rgent  - can't do anything else until this is done.
 [H]igh    - must be done as soon as possible.
 [M]edium  - should be done fairly soon.
 [L]ow     - not needed soon.
 [V]erylow - do when there is nothing else to do.
 [D]ubious - not sure that we want to do this.

[D]ifficulty values range from 1 (easy) to 5 (hard)

[W]ho the task is assigned to: [C]hris [R]ichard [O]lly

";
    $table_begin = 
" | | |   |             |\n" .
"P|D|W|Rel|Area         | Task\n" .
"-+-+-+---+-------------+-------------------------------------------------------\n";
    $table_end = "";
    $postamble = "";
} else {
    $preamble =
'<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN">
<HTML>
<HEAD>
<META NAME="Created" CONTENT="17/10/00">
<META NAME="Created By" CONTENT="|l|e|n|s|i|s|">
<META NAME="Resource Type" CONTENT="Document">
<META NAME="Copyright" CONTENT="(c)2001 xapian">
<META NAME="Description" CONTENT="XAPIAN developers zone to do list">
<META NAME="Keywords" CONTENT="XAPIAN to do list required tasks">
<TITLE>xapian developers zone - to do list</TITLE>
<!--LINK rel=stylesheet type="text/css" HREF="xapianweb.css"-->
</HEAD>

<!--#set var="menupos_1" value="developers" -->
<!--#set var="menuselection" value="todo" -->
<!--#set var="topdir" value="../../" -->
<!--#include virtual="${topdir}tmpl/nav.shtml" -->

<BODY BACKGROUND="../../art/bg.gif" BGCOLOR="#FFFFFF" TEXT="#000000" LINK="#333399" ALINK="#6666CC" VLINK="#663366" TOPMARGIN=0 LEFTMARGIN=0 MARGINHEIGHT=0 MARGINWIDTH=0>

<!--#include virtual="${topdir}tmpl/pagebegin.shtml" -->
';
    $table_begin =
"<TABLE BORDER=1>
<TR>
<TD>Priority</TD>
<TD>Difficulty</TD>
<TD>Area</TD>
<TD>Task</TD>
<TD>Release</TD>
<TD>Owner</TD>
</TR>
";
    $table_end = "</TABLE>\n";
    my $date = `date '+%Y/%m/%d %H:%M:%S'`;
    chomp $date;
    my $author = 'a script';

    $postamble =
"<!-- FOOTER \$ Author: $author \$ \$ Date: $date \$ \$ Id: \$ -->\n" .
'<!--#include virtual="${topdir}tmpl/pageend.shtml" -->' .
"\n</BODY>\n</HTML>\n";
}

print $preamble;

my $p1 = new XML::Parser(Handlers => {Start => \&handle_start,
                                      End   => \&handle_end,
				      Char  => \&handle_char});

my @items;
print $table_begin;
$p1->parsefile($ARGV[0]);

displayitems();
print $table_end;

print $postamble;

sub handle_start {
    my $xpat = shift;
    my $tag = shift;
    $tag eq 'todo' and return;
    $tag eq 'todoitem' or die "unknown element <$tag ...>\n";
    %field = ('owner' => '', 'field' => '');
    while (scalar @_) {
	my $name = shift;
	my $value = shift;
	$field{$name} = $value;
    }
}

sub display_item {
    my %field = %{shift()};
    if ($text) {
        print join "|", map substr("$field{$_} ", 0, 1), qw(priority difficulty owner);
        my $f = Text::Format->new({columns => 54, firstIndent => 0});
	my @tasklines = $f->format($field{'task'});
        my $f2 = Text::Format->new({columns => 13, firstIndent => 0});
	my @arealines = $f2->format($field{'area'});
        my $f3 = Text::Format->new({columns => 3, firstIndent => 0});
	my @releaselines = $f3->format($field{'release'});
	my $prefix = '';
	my $lines = scalar @tasklines;
	$lines = scalar @arealines if scalar @arealines > $lines;
	$lines = scalar @releaselines if scalar @releaselines > $lines;
	for (1..$lines) {	
	    my $area = shift @arealines || '';
	    chomp $area;
	    my $task = shift @tasklines || '';
	    chomp $task;
	    my $release = shift @releaselines || '';
	    chomp $release;
	    printf "%s|%-3s|%-13s| %s\n", $prefix, $release, $area, $task;
	    $prefix = " | | ";
	}
        print "-+-+-+---+-------------+-------------------------------------------------------\n";
    } else {
        my $colour = '#' . ($priority_colours{$field{priority}} || 'ffffff');
	print <<EOT;
<TR VALIGN=top>
<TD BGCOLOR="$colour" VALIGN=top>$priority{$field{priority}}</TD>
<TD BGCOLOR="$colour" VALIGN=top>$field{difficulty}</TD>
<TD BGCOLOR="$colour" VALIGN=top>$field{area}</TD>
<TD BGCOLOR="$colour" VALIGN=top>$field{task}</TD>
<TD BGCOLOR="$colour" VALIGN=top>$field{release}</TD>
<TD BGCOLOR="$colour" VALIGN=top>$field{owner}</TD>
</TR>
EOT
    }
}

sub handle_end {
    my $xpat = shift;
    my $tag = shift;
    $tag eq 'todoitem' or return;
    if ($filter_release) {
	if ($filter_release eq '*') {
	    if ($field{'release'} eq '') { return; }
	} else {
	    if ($filter_release ne $field{'release'}) { return; }
	}
    }
    my %fieldcopy = %field;
    push(@items, \%fieldcopy);
}

sub cmprelease {
    my $r1 = $$a{'release'};
    my $r2 = $$b{'release'};
    $r1 = "9999999" if $r1 eq "";
    $r2 = "9999999" if $r2 eq "";
    if ($r1 eq '*') {
	return ($r2 eq '*') ? 0 : -1;
    }
    if ($r2 eq '*') {
	return 1;
    }
    $r1 <=> $r2;
}

sub cmppriority {
    my $r1 = $$a{'priority'};
    my $r2 = $$b{'priority'};
    $r1 = $priorityorder{$r1};
    $r2 = $priorityorder{$r2};
    $r2 <=> $r1;
}

sub cmparea {
    my $r1 = $$a{'area'};
    my $r2 = $$b{'area'};
    $r1 cmp $r2;
}

sub cmptask {
    my $r1 = $$a{'task'};
    my $r2 = $$b{'task'};
    $r1 cmp $r2;
}

sub cmpitems {
    my $c = cmprelease();
    return $c if $c != 0;
    $c = cmppriority();
    return $c if $c != 0;
    $c = cmparea();
    return $c if $c != 0;
    cmptask();
}

sub displayitems {
    my $fieldref;
    foreach $fieldref (sort cmpitems @items) {
	display_item($fieldref);
    }
}

sub handle_char {
    my ($xpat, $string) = @_;
    $field{task} .= $string;
}
