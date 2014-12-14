# 
# Trantor Operating System
# Copyright (C) 2014 Raghu Kaippully
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#
# Makefile for Trantor documentation
#

html: trantor.asciidoc
	a2x -f chunked trantor.asciidoc
	mv trantor.chunked/* .
	rmdir trantor.chunked

pdf: trantor.asciidoc
	a2x -f pdf trantor.asciidoc

clean:
	$(RM) *.html docbook-xsl.css trantor.pdf
