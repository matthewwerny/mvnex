// The plugin installs and its generated help goal runs.
def log = new File(basedir, 'build.log').text
assert log.contains('ex-maven-plugin')
assert log.contains('ex:help')
