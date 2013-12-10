austese_install
===============

This is an all-in-one installer for AustESE.

AustESE is a tool set for creating digital scholarly editions. A digital 
scholarly edition (DSE) is a digital edition of a historical written 
work. It consists of a set of "digital objects", which are mostly 
transcriptions of the text of the documents plus facsimiles of the 
pages. A DSE supports five basic operations: 

1) cataloguing via various metadata formats. AustESE maintains metadata 
based on dublin core for each digital object.

2) Markup via standoff properties rather than embedded tags, as this 
allows properties to overlap. they are stored in a custom JSON format 
called STIL (standard interval language)

3) comparison via multi-version (MVD) documents. The plain text and 
markup of documents are stored separately and are merged into a single 
multi-version graph form. This allows differences to be quickly read 
rather than computed from the sources.

4) commentary via Open Annotations, using Lorestore, from the Lore 
annotation tool.

5) citations via standoff properties and a citation service. This is 
currently not done for AustESE.

AustESE is really a kind of back-end or project manager. Its purpose is 
to create digital scholarly editions, not to be one.

Installation 
------------ 

The installer is for Ubuntu/Debian only. It downloads all the required 
deb packages, creates a Drupal directory in the user's web-root, and 
uploads a basic database (austese) to MySql. it installs calliope and 
Lorestore in Tomcat 7. The user can then create projects in the Drupal 
CMS for building DSEs. From the project, completely customised 
front-ends or actual DSE websites can be constructed, requiring only the 
standard web technologies of HTML, Javascript and CSS.

AustESE is based on the Drupal content management
system, which requires PHP. The calliope and Lorestore services requires Java.
Calliope currently requires a JVM of around 2GB and can only merge 
text files of about 50-75K per version, but images have 
no size limitation. 

Calliope performs the following operations: comparison of versions, 
formatting of text and markup as HTML, importing and exporting files in 
HTML, XML, TEXT and MVD. Currently calliope can only import, not export 
XML, but this is expected to be added soon.

Lorestore permits the creation, reading, updating, and deletion (CRUD) 
of OA annotations in a variety of formats including JSON-LD and RDF/XML 
Trix, Turtle, TriG.

The text to image linking tool (TILT) is currently unfinished, but will 
be completed soon. It will allow the semi-automated linking of text 
selections and image areas at the word-level using text recognition.

AustESE has been funded by the Australian government's NECTaR initiative 
and is based at the University of Queensland's eResearch laboratory.


To Do's
-------

NMergeC will remove the file limit and reduce the memory footprint. XML 
export needs to be added. The citation service is not done. Text to 
image linking is incomplete. As yet there is no manual.
