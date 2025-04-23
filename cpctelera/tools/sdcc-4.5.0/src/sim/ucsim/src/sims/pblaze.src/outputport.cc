/*
 * Copyright (C) 2012-2013 Jiří Šimek
 * Copyright (C) 2013 Zbyněk Křivka <krivka@fit.vutbr.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include <map>
#include <list>

// local
#include "pblazecl.h"
#include "outputportcl.h"

#include "tinyxml.h"


cl_output_port::cl_output_port(class cl_uc *auc) : cl_hw(auc, HW_PORT, 0, "output_port")
{
}


bool
cl_output_port::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
{
    con->dd_printf("Output port is read-only\n", id);
    return true; // handled
}

void
cl_output_port::set_help(class cl_console_base *con)
{
}

void
cl_output_port::print_info(class cl_console_base *con)
{
  con->dd_printf("Value on output port: %2X\n", value);
}



void
cl_output_port::add_output(int port, long tick, t_mem value) {
  if (outputs.find(port) == outputs.end()) {
    outputs.insert(make_pair(port, list<struct output>()));
  }

  struct output o;
  o.tick = tick;
  o.value = value;

  outputs[port].push_back(o);
}

void
cl_output_port::print_outputs(char *file_name) {
  /*
  if (file_name == NULL) {
    for (output_map::iterator it = outputs.begin(); it != outputs.end(); it++) {
      application->dd_printf("port %d:\n", it->first);

      for (list<struct output>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
        application->dd_printf("\ttick %d: %2x\n", (*it2).tick, (*it2).value);
      }
    }
  }
  else {*/
    char buffer[8];

    TiXmlDocument doc;
    TiXmlElement *root;
    TiXmlElement *port, *element;
    TiXmlText * text;

    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
    doc.LinkEndChild( decl );
    root = new TiXmlElement( "outputs" );
    doc.LinkEndChild( root );


    for (output_map::iterator it = outputs.begin(); it != outputs.end(); it++) {
      port = new TiXmlElement( "port" );
      port->SetAttribute("id", it->first);
      root->LinkEndChild( port );

      for (list<struct output>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
        element = new TiXmlElement( "output" );
        element->SetAttribute("tick", (*it2).tick);
        sprintf(buffer, "%2x", (*it2).value);
        text = new TiXmlText(buffer);
        element->LinkEndChild( text );
        port->LinkEndChild( element );
      }
    }


  if (file_name == NULL) {
    TiXmlPrinter printer;
    doc.Accept(&printer);
    application->dd_printf("%s", printer.CStr());
  }
  else {
    doc.SaveFile( file_name );
  }
}

/* End of pblaze.src/outputport.cc */
