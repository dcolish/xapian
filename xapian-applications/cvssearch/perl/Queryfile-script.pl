#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html with frames linking to other frames:
# 1. match.cgi
# 2. source.cgi
# 3. comments.cgi
#
# Things needed to pass to this script
# 1. dump file
# 2. file id (rootx project id)
# 
# Structure:
# 1. create a frame structure
# 2. pass dump and id to all other frames
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 25 2001
#-------------------------------------------------------------------

use CGI ':all';
use Cvssearch;

#-------------------
# path variables
#-------------------
$CVSDATA = $ENV{"CVSDATA"}; # path where database content file is stored
$source = "./Source.cgi";
$match = "./Match.cgi";
$comments = "./Comments.cgi";
$top = "./Top.cgi";

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";

if(param()){
	$dump = param("dump");
	$id = param("id");
	$displayname = param("displayname");
	$id = Cvssearch::encode($id);
	$tempdisplay = $displayname;
	$displayname = Cvssearch::encode($displayname);
	$passparam1 = "?id=$id&dump=$dump";
	$passparam2 = "?id=$id&dump=$dump&displayname=$displayname";

print <<_HTML_;	
<head>
<title>$tempdisplay</title>
</head>
<frameset rows=\"45,*\">
	<frame name=top src=$top$passparam2>
	<frameset rows=\"50%,50%\">
		<frame name=m src=$match$passparam1>
		<frame name=s src=$source$passparam1>
	</frameset>
</frameset>
<noframes>
	<body bgcolor=\"#FFFFF0\">
	sorry this page requires frame to be displayed
	</body>
</noframes>
_HTML_
 	
}else{
	print "need to pass query and root as parameters"
}
print "</html>";
