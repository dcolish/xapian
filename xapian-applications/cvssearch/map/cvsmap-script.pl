# -* perl *-

@file_types= qw(cc h cpp c C);

# ------------------------------------------------------------
# path where all our files are stored.
# if $CVSDATA is not set, use current directory instead.
# ------------------------------------------------------------
$CVSDATA = $ENV{"CVSDATA"}; 

if($CVSDATA) {
}else{
    print STDERR "WARNING: \$CVSDATA not set! use current directory.\n";
    $CVSDATA = ".";
}


if ($#ARGV < 0) {
    usage();
}

$i = 0;

while ($i<=$#ARGV) {
    if (0) {
    } elsif ($ARGV[$i] eq "-t") {
        $i++;
        if ($i<=$#ARGV) {
            @file_types = split(/\" /, $ARGV[$i]);
            $i++;
        }
    } elsif ($ARGV[$i] eq "-f") {
        $i++;
        if ($i<=$#ARGV) {
            $apps_file = $ARGV[$i];
            $i++;
        }
    } elsif ($ARGV[$i] eq "-h") {
        usage();
    } else {
        @modules = (@modules, $ARGV[$i]);
        $i++;
    }
}

$list_file="$CVSDATA/.list";
$time_file="$CVSDATA/time";

$delta_time = 0;
unlink $time_file;
unlink $list_file;

print "APPFILE $apps_file\n";

open(APPS, "<$apps_file");
open(TIME, ">$time_file");
    
while ($app_path = <APPS>) {
    chomp($app_path);
    print "APP $app_path\n";
    $app_name = $app_path;
    $app_name =~ tr/\//\_/;
    print "APP NAME $app_name\n";
    $app_name = "$CVSDATA/$app_name";

    if ($app_path ne "" ) {
        system ("cvs checkout $app_path");
        print "$app_path\n";
        $found_files = 0;
        open(LIST, ">$list_file") || die "cannot create temporary file list\n";
        for ($i = 0; $i <= $#file_types; ++$i) {
            open(FIND_RESULT, "find $app_path -name \"*.$file_types[$i]\"|");
            while (<FIND_RESULT>) {
                $found_files = 1;
                print LIST $_;
            }
            close(FIND_RESULT);
        }
        close(LIST);
        
        if ($found_files) {
            print TIME "$app_path", "\n";
            print TIME "Started  @ ", `date`;
            $start_date = time;
            system ("cvsmap -i $list_file -f1 $app_name.comments -f2 $app_name.offset");
            print TIME "Finished @ ", `date`;
            $delta_time += time - $start_date;
            print TIME "\n";
        }
        system ("rm -rf $app_path");
    }
}
print TIME "Operation Time: $delta_time Seconds \n";
close(APPS);
close(REC);

sub usage() {
    print << "EOF";
cvsbuild 0.1 (2001-2-22)
Usage $0 [Options]
        
Options:
    -t file_types   specify file types of interest. e.g. -t html java
                    will only do the line mapping for files with extension
                    .html and .java; default types include: c cc cpp C h.
    modules       a list of modules to built, e.g. koffice/kword  kdebase/konqueror
    -f app.list   a file containing a list of modules
    -h            print out this message
EOF
exit 0;
}
