This project builds to the folder:
  %appdata%\zenova\mods
  
That folder is where all mods for the launcher are stored, and for
convience I set this project to build to that location by default.

If you're having issues building the project, make sure there is a
folder that exists at the build destination. Later we will include
a custom build step to automaically create the folder so this issue
doesn't occur.
