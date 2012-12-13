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

'''
Created on 10 Dec 2012
'''

import polib
from lxml.etree import Element, ElementTree, SubElement

class PoFile:
    def __init__(self, infile):
        self.original = polib.pofile(infile)
        self.po = polib.pofile(infile)
        self._add_keyword_entries()

    def _add_keyword_entries(self):
        new_entries = []
        
        for entry in self.po.translated_entries():
            new_entry = polib.POEntry(
                msgid=u'hud-keywords:{}'.format(entry.msgid),
                msgstr=u'',
                occurrences=entry.occurrences
            )
            new_entries.append(new_entry)

        for entry in new_entries:
            self.po.append(entry)
            
    def save(self, path):
        self.po.save(path)
        
    def save_xml(self, path):
        keyword_mapping = Element('keywordMapping')
        
        for entry in self.original.translated_entries():
            mapping = SubElement(keyword_mapping, 'mapping', original=entry.msgid)
            SubElement(mapping, 'keyword', name='')
            SubElement(mapping, 'keyword', name='')
            
        ElementTree(keyword_mapping).write(path, encoding='utf-8', xml_declaration=True, pretty_print=True)
