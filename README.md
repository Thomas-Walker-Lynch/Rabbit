 # Harmony

## About

This is an RT project skeleton. There are a few files from a tentative project here, that service as either an example or hinderence.

Source one of these evironment files depending on the role being played when entering the project:

- env_developer - for code developer role
- env_tester    - for tester role
- env_toolsmith - for the toolsmith role

developers work out of the 'developer' directory
testers work out of the 'tester' direcgtory
toolsmthis set up 'tool_shared' and the various env scripts.

document/ - for project documents
developer/document/ - documents specifically concerning development
developer/tool/ - tools specific for development

tool_shared/ for tools shared by mulitple roles.
tool_shared/third_party  for third party tools. For example, if you are going to install Python, put the virtual environment in this directoy under the name 'Python' and set a search path to it under `env_developer` or whereever it gets used from.

See other projects for examples.  Ariadne or Mosaic projects might be good examples.  Note we no longer using the 🖉 to mark authored content.

## License

Harmoy is not distributed with an MIT license. However, projects that
use the Harmony skeleton might be distrbuted under other licenses. See the directory document/license for a nonexculsive list of other licenses that a project that mekes use of the Harmony skeleton might make use of.