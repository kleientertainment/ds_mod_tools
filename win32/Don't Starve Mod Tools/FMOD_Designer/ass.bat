@rem Have to use %* here because the path provided may have spaces
@rem and quoting it gives "path with spaces"\fmod_designer.exe, which fails.

ftype FMODDesignerProject=%*\fmod_designer.exe "%%1" %%*
assoc .fdp=FMODDesignerProject
ftype FMODEventFile=%*\fmod_eventplayer.exe "%%1" %%*
assoc .fev=FMODEventFile
