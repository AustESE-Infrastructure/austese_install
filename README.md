austese_install
===============

All-in-one installer for AustESE tools and calliope.

AustESE is a tool set for creating digital scholarly 
editions. It is really a kind of back-end or project 
manager. Its purpose is to create digital scholarly 
editions, not to be one.

This installer is for Ubuntu/Debian only. It installs all
the required deb packages, then creates a blank AustESE
Drupal directory in the user's web-directory, and then
uploads a basic database (austese) to MySql. Finally
it installs the calliope web-service to complete the 
set. The user can then create projects for building DSEs.
Completely customised websites can then be constructed, 
based on the resources managed by the AustESE project 
on any web-based platform, requiring only the standard 
web technologies of HTML, Javascript and CSS. 

AustESE is based on the Drupal content management
system, and requires an Ubuntu or Debian Linux server
running Java with a JVM of around 2GB. Text files are 
limited to around 50-75K per version, but images have 
no size limitation. AustESE can import plain text, XML 
or HTML, and export to HTML, MVD and text (with JSON 
markup). It is hoped to add XML export later.

The text to image linking tool (TILT) is unfinished, 
but will be completed soon. AustESE is planned as a 
general tool set for creating any kind of digital 
scholarly edition and has been funded by the Australian 
government's NECTaR initiative.

As yet there is no manual.
