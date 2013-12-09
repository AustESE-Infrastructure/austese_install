austese_install
===============

This is an all-in-one installer for AustESE.

AustESE is a tool set for creating digital scholarly 
editions. It is really a kind of back-end or project 
manager. Its purpose is to create digital scholarly 
editions, not to be one.

This installer is for Ubuntu/Debian only. It installs all
the required deb packages, creates a blank AustESE
Drupal directory in the user's web-directory, and
uploads a basic database (austese) to MySql. Finally
it installs the calliope web-service to complete the 
set. The user can then create projects for building 
DSEs. It supports the five key operations of a DSE: 
1) cataloguing via various metadata formats 2) Markup 
via STIL (standard interval language) standoff 
properties which allows full overlap 3) comaprison 
via multi-version (MVD) documents 4) commentary via 
Lorestore and OA annotations and 5) citations via 
standoff properties and a citation service (yet to do).

Once a project has been created, completely customised 
front-ends or actual DSE websites can be constructed, 
requiring only the standard web technologies of HTML, 
Javascript and CSS. 

AustESE is based on the Drupal content management
system, and requires an Ubuntu or Debian Linux server
running Java with a JVM of around 2GB. Text files are 
limited to around 50-75K per version, but images have 
no size limitation. Two other components are needed: 
the calliope web-service for handling formatting and 
MVDs, and Lorestore for annotations. Both run inside 
Tomcat. AustESE can import plain text, XML or HTML, 
and export to HTML, MVD and text (with JSON markup). 
It is hoped to add XML export later.

The text to image linking tool (TILT) is unfinished, 
but will be completed soon. AustESE is planned as a 
general tool set for creating any kind of digital 
scholarly edition and has been funded by the Australian 
government's NECTaR initiative.

To Do's
NMergeC will remove the file limit and reduce the memory 
footprint. XML export needs to be added. The citation 
service is not done. Text to image linking is incomplete. 
As yet there is no manual. But a lot of it works.
