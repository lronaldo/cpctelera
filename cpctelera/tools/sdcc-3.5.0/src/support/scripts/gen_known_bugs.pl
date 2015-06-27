# gen_known_bugs.pl - generate knownbugs.html
#
# Copyright (c) 2007-2013 Borut Razem
#
# This file is part of sdcc.
#
#  This software is provided 'as-is', without any express or implied
#  warranty.  In no event will the authors be held liable for any damages
#  arising from the use of this software.
#
#  Permission is granted to anyone to use this software for any purpose,
#  including commercial applications, and to alter it and redistribute it
#  freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
#  2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
#  3. This notice may not be removed or altered from any source distribution.
#
#  Borut Razem
#  borut.razem@siol.net

use strict;
use warnings;

use LWP::Simple;
use HTML::TreeBuilder;


# trim function to remove whitespace from the start and end of the string
sub trim($)
{
  my $string = shift;
  $string =~ s/^\s+//;
  $string =~ s/\s+$//;
  return $string;
}


my @headerList = ('#', 'Summary', 'Owner', 'Creator', 'Created', 'Priority');
my $nFields = $#headerList + 1; # Number of colums is number of header fields + 1 for "Select Columns" icon

# check if the line is a correct header
sub is_header($)
{
  my ($line) = @_;
  
  if (ref($line)) {
    my @headers = $line->look_down('_tag', 'th');
    foreach my $header (@headerList) {
      my $found = 0;
      foreach (@headers) {
        my $content = trim($_->as_text());
        if ($content =~ /\Q$header\E/i) {
          $found = 1;
          last;
        }
      }
      if (!$found) {
        return 0;
      }
    }
    return 1;
  }
  return 0;
}


# check if the line has correct number of fields
sub has_all_fields($)
{
  my ($line) = @_;
 
  my @len = $line->look_down('_tag', 'td');
  return $#len == $nFields;
}

# convert to ISO date
sub date_to_iso($)
{
  my %months = (
    'Jan', '01',
    'Feb', '02',
    'Mar', '03',
    'Apr', '04',
    'May', '05',
    'Jun', '06',
    'Jul', '07',
    'Aug', '08',
    'Sep', '09',
    'Oct', '10',
    'Nov', '11',
    'Dec', '12'
  );

  my ($date) = @_; #Mon Mar 14, 2011 10:42 AM UTC
  my (undef, $month, $day, $year) = split(' ' , $date);
  $day =~ s/^(\d+),$/$1/;
  return $year . '-' . $months{$month} . '-' . sprintf('%02d', $day);
}

# process a line
sub process_line($)
{
  my ($line) = @_;

  my $i = 0;
  foreach ($line->look_down('_tag', 'td')) {
    if (!defined($headerList[$i])) {
      # don't print columns which are not in the header list
      $_->delete();
    }
    elsif ($headerList[$i] eq 'Summary') {
      # convert relative to absolute href in the 'Summary' field
      foreach ($_->look_down('_tag', 'a')) {
        my $attr = $_->attr('href');
        if (defined($attr)) {
          $_->attr('href', 'https://sourceforge.net' . $attr);
        }
      }
    }
    elsif ($headerList[$i] eq 'Owner' || $headerList[$i] eq 'Creator') {
      $_->normalize_content();
    }
    elsif ($headerList[$i] eq 'Created') {
      my $date = $_->look_down('_tag', 'span')->attr('title');
      $_->delete_content();
      $_->push_content(date_to_iso($date));
    }
    elsif ($headerList[$i] eq 'Priority') {
      my @content = $_->content_list();
      my $v = 0;
      $v = $content[0] if (0 == $#content);
      $_->{'_parent'}->{'class'} = 'p' . $v;
    }
    ++$i;
  }
  $line->delete_ignorable_whitespace();
}


# process the HTML page
sub process_page($)
{
  my ($html) = @_;

  # create HTML tree from the page
  my $tree = HTML::TreeBuilder->new();
  $tree->parse($html);

  # find table with the required header
  my $lines = 0;
  foreach my $table ($tree->look_down('_tag', 'table')) {
    my $thead = $table->look_down('_tag', 'thead');
    if (is_header($thead)) {
      my $tbody = $table->look_down('_tag', 'tbody');
      my @lines = $tbody->content_list();

      # process the lines in table
      # if they have required number of fields
      foreach my $line (@lines) {
        if (ref($line) && has_all_fields($line)) {
          # process a line
          process_line($line);
          # and print it
          print($line->as_HTML(undef, '  '));
          ++$lines;
        }
      }
    }
  }

  $tree->delete;
  
  return $lines;
}


# print HTML header
sub print_header($)
{
  my ($version) = @_;

  print <<EOF;
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en">
<!--
This file is generated automagicaly by gen_known_bugs.pl script.
-->
  <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8" />
    <title>SourceForge.net: Known Bugs</title>
    <style type="text/css">
      .p1 {background-color: #9ff;}
      .p2 {background-color: #cff;}
      .p3 {background-color: #9fc;}
      .p4 {background-color: #cfc;}
      .p5 {background-color: #cf9;}
      .p6 {background-color: #ffc;}
      .p7 {background-color: #ff9;}
      .p8 {background-color: #fc9;}
      .p9 {background-color: #fcc; color: #300;}
    </style>
  </head>
  <body>
    <h2>Small Device C Compiler - Release $version Known Bug List</h2>
    <ul>
      <li><a href="http://sdcc.sourceforge.net">Home&nbsp;Page</a></li>
      <li class="selected"><a href="http://sourceforge.net/p/sdcc/bugs/">Current Bugs</a></li>
    </ul>
    <table width="100%" border="0" cellspacing="2" cellpadding="3">
      <tr bgcolor="#ffffff">
EOF

  foreach my $header (@headerList) {
    # don't print Status and Resolution columns
    if ($header ne 'Status' && $header ne 'Resolution') {
      print('        <td align="center"><font color="#000000"><b>' . $header . "</b></font></td>\n");
    }
  }
  print("      </tr>\n");
}


# print HTML footer
sub print_footer($)
{
  my ($lines) = @_;

  print <<EOF;
    </table>
    <p><b>Priority Colors:</b></p>
    <table border="0">
      <tr>
        <td class="p1">1</td>
        <td class="p2">2</td>
        <td class="p3">3</td>
        <td class="p4">4</td>
        <td class="p5">5</td>
        <td class="p6">6</td>
        <td class="p7">7</td>
        <td class="p8">8</td>
        <td class="p9">9</td>
      </tr>
    </table>
  </body>
 <p><b>Number of open bugs: $lines</b></p>
</html>
EOF
}


# main procedure
{
  my $url = "http://sourceforge.net/p/sdcc/bugs/?limit=%d&page=%d&sort=ticket_num+desc&q=%%7B%%22status%%22%%3A+%%7B%%22%%24nin%%22%%3A+%%5B%%22closed-invalid%%22%%2C+%%22closed-later%%22%%2C+%%22closed-accepted%%22%%2C+%%22closed-duplicate%%22%%2C+%%22closed-out-of-date%%22%%2C+%%22closed-fixed%%22%%2C+%%22closed-rejected%%22%%2C+%%22closed-remind%%22%%2C+%%22closed-works-for-me%%22%%2C+%%22closed%%22%%2C+%%22closed-wont-fix%%22%%2C+%%22closed-postponed%%22%%5D%%7D%%7D&columns-0.name=ticket_num&columns-0.sort_name=ticket_num&columns-0.label=Ticket+Number&columns-0.active=on&columns-1.name=summary&columns-1.sort_name=summary&columns-1.label=Summary&columns-1.active=on&columns-2.name=_milestone&columns-2.sort_name=custom_fields._milestone&columns-2.label=Milestone&columns-3.name=status&columns-3.sort_name=status&columns-3.label=Status&columns-4.name=assigned_to&columns-4.sort_name=assigned_to_username&columns-4.label=Owner&columns-4.active=on&columns-5.name=reported_by&columns-5.sort_name=reported_by&columns-5.label=Creator&columns-5.active=on&columns-6.name=created_date&columns-6.sort_name=created_date&columns-6.label=Created&columns-6.active=on&columns-7.name=mod_date&columns-7.sort_name=mod_date&columns-7.label=Updated&columns-8.name=labels&columns-8.sort_name=labels&columns-8.label=Labels&columns-9.name=_priority&columns-9.sort_name=_priority&columns-9.label=Priority&columns-9.active=on";

  if ($#ARGV != 0) {
    printf("Usage: gen_known_bugs.pl <version>\n");
    exit(1);
  }

  my $limit = 100;

  # get the SDCC version number from command line
  my $version = $ARGV[0];

  # print HTML header
  print_header($version);

  # get pages from SF bug tracker
  # and process them
  my $page = 0;
  my $lines;
  while (my $linesRead = process_page(get(sprintf($url, $limit, $page)))) {
    $lines += $linesRead;
    ++$page;
  }

  # print HTML footer
  print_footer($lines);

  exit(0);
}
