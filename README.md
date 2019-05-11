# Jörmungandr

Jörmungandr (a.k.a. Jormungandr) is a project for scheduling a queue of tasks
onto a limited set of devices.

Use "jormungandr launch /tmp/Server" to launch a server using the directory
"/tmp/Server". The "--numslots" and "-s" options can be used to specify how many
commands the server can run at once.

Once a server is launched, use "jormungandr schedule /tmp/Foo -- cmd arg1 arg2"
to have the "/tmp/Foo" server schedule the command "cmd" for execution, with the
given arguments. Scheduled commands are stored in a queue, so the scheduled
commands are executed on a first-come-first-served basis; however, if the
priority flag ("--priority" or "-p") is present when scheduling a command, that
command will be placed at the front of the queue instead of the back.



## Getting Setup for Development

Add git hooks: git config core.hooksPath ./Scripts/GitHooks/

Manually installing Unity and related scripts:

* verify that ruby is installed

* make sure that the "RUBY" variable in Tmp/installUnity.bash has the correct
  path for your ruby install

* Run Tmp/installUnity.bash, taking care to follow the instructions shown at the
  end.

* After running, verify that the ruby command installation was a success by
  calling the command "generate_test_runner.rb". As of this writing, calling it
  with no arguments causes it to fail and produce output explaining how to use
  it.

* Run `make` in the root project directory.

  * If make fails even though you have correctly followed the previous
    instructions:

      It's probably because your system finds includes files differently than
      mine; try tweaking Tmp/installUnity.bash to install the scripts, headers,
      and shared library file in a more appropriate location
