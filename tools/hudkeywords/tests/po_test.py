# Copyright (C) 2005-2012 Canonical Ltd
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


import unittest, os
from hudkeywords.po import PoFile
from tempfile import mkstemp

class TestPoFile(unittest.TestCase):

    INPUT = r'''# SOME DESCRIPTIVE TITLE.
# Copyright (C) YEAR THE PACKAGE'S COPYRIGHT HOLDER
# This file is distributed under the same license as the PACKAGE package.
# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.
# 
#, fuzzy
msgid ""
msgstr ""
"Project-Id-Version: hellogt 1.2\n"
"Report-Msgid-Bugs-To: \n"
"POT-Creation-Date: 2012-11-21 14:24+0000\n"
"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n"
"Last-Translator: FULL NAME <EMAIL@ADDRESS>\n"
"Language-Team: LANGUAGE <LL@li.org>\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=CHARSET\n"
"Content-Transfer-Encoding: 8bit\n"
"Language: \n"

#: hellogt.cxx:16
msgid "hello, world!"
msgstr ""

#: hellogt.cxx:17 hellogt.cxx:19
msgid "another string"
msgstr ""

#: hellogt.cxx:20
msgid "another string 2"
msgstr ""
'''
    
    PO_OUTPUT = INPUT + r'''
#: hellogt.cxx:16
msgid "hud-keywords:hello, world!"
msgstr ""

#: hellogt.cxx:17 hellogt.cxx:19
msgid "hud-keywords:another string"
msgstr ""

#: hellogt.cxx:20
msgid "hud-keywords:another string 2"
msgstr ""
'''

    XML_OUTPUT = r'''<?xml version='1.0' encoding='UTF-8'?>
<keywordMapping>
  <mapping original="hello, world!">
    <keyword name="Keyword1"/>
    <keyword name="Keyword2"/>
  </mapping>
  <mapping original="another string">
    <keyword name="Keyword1"/>
    <keyword name="Keyword2"/>
  </mapping>
  <mapping original="another string 2">
    <keyword name="Keyword1"/>
    <keyword name="Keyword2"/>
  </mapping>
</keywordMapping>
'''

    def test_save(self):
        po = PoFile(self.infile)
        po.save(self.outfile)
        with os.fdopen(self.outhandle, 'r') as f:
            self.assertEqual(f.read(), TestPoFile.PO_OUTPUT)

    def test_save_xml(self):
        po = PoFile(self.infile)
        po.save_xml(self.outfile)
        with os.fdopen(self.outhandle, 'r') as f:
            self.assertEqual(f.read(), TestPoFile.XML_OUTPUT)

    def setUp(self):
        self.inhandle, self.infile = mkstemp()
        with os.fdopen(self.inhandle, 'w') as f:
            f.write(TestPoFile.INPUT)
        self.outhandle, self.outfile = mkstemp()
        
    def tearDown(self):
        os.remove(self.infile)
        os.remove(self.outfile)