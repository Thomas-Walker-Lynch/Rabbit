# This is a document, not a script
# There are more docs on IDEs, directory structure, etc.
# Note especially the workflow document.

#1. In a login shell

  # enter the project environment as a developer
  > cd Rabbit
  > . env_developer

  # run your IDE 
  > emacs

# 2. inside of an emacs shell, or IDE build scrit

  # do you edits
  # run local test experiments

  > make all
  > make release


  # 3. In env_tester run more thorough test suite.  When satisfied make a release branch and tag it.
  #    Release branches have consecutive major release numbers.
       
  
