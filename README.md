# Jörmungandr

Jörmungandr (a.k.a. Jormungandr) is a project for scheduling a queue of tasks
onto a limited set of devices.

Use `jormungandr launch /tmp/Server` to launch a server that uses "/tmp/Server"
as its working directory. The server's log files, among other things, are stored
in this directory. The `--numslots` option can be used to specify how many
resources are availible to the server.

Once a server is launched, you can use `jormungandr schedule /tmp/Foo -- cmd
arg1 arg2` to have the specified server schedule the command `cmd` for
execution, with the given arguments. Scheduled commands are stored in a queue,
so they are executed on a first-come-first-served basis; however, if the
priority option (`--priority`) is present when scheduling a command, that
command will be placed at the front of the queue instead of the back. You can
specify how many resources a command requires using the `--slotsuse` option.

Much like [Task Spooler](http://vicerveza.homeunix.net/~viric/soft/ts/),
Jörmungandr ensures that the total number of resources required by all of the
currently running jobs does not exceed the maximum. Unlike Task Spooler,
however, the list of resources are enumerated and a variable is set in the
executed command's environments, telling them exactly which resources they
should use. With the aid of a helper script, this could be used to, for example,
guarantee that a sequence of hundreds of machine learning scripts do not attempt
to use the same GPU.

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
