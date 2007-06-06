<?php
/* PHP4 script to index each paragraph of a text file as a Xapian document.
 *
 * Copyright (C) 2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

if (php_sapi_name() != "cli") {
    print "This example script is written to run under the command line ('cli') version of";
    print "the PHP interpreter, but you're using the '".php_sapi_name()."' version\n";
    exit(1);
}

include "php4/xapian.php";

// PHP < 4.3.0 only sets $argc and $argv if 'register_globals' is on.
if (!isset($argc)) $argc = $_SERVER['argc'];
if (!isset($argv)) $argv = $_SERVER['argv'];

if ($argc != 2) {
    print "Usage: {$argv[0]} PATH_TO_DATABASE\n";
    exit(1);
}

// Open the database for update, creating a new database if necessary.
$database = new XapianWritableDatabase($argv[1], Xapian_DB_CREATE_OR_OPEN);
if (!$database) {
    print "Couldn't create or open database '{$argv[1]}' for indexing\n";
    exit(1);
}

$indexer = new XapianTermGenerator();
$stemmer = new XapianStem("english");
$indexer->set_stemmer($stemmer);

$para = '';
$lines = file("php://stdin");
foreach ($lines as $line) {
    $line = rtrim($line);
    if ($line == "" && $para != "") {
	// We've reached the end of a paragraph, so index it.
	$doc = new XapianDocument();
	$doc->set_data($para);

	$indexer->set_document($doc);
	$indexer->index_text($para);

	// Add the document to the database.
	$database->add_document($doc);

	$para = "";
    } else {
	if ($para != "") {
	    $para .= " ";
	}
	$para .= $line;
    }
}

// Set the database handle to Null to ensure that it gets closed
// down cleanly or unflushed changes may be lost.
$database = Null;
?>
