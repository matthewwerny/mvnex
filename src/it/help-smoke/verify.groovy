import static com.sebas3261.ex.it.ItSupport.*

// The generated help lists every goal and shows the plugin version.
def log = new File(basedir, 'build.log')
assertContains(log, "ex-maven-plugin ${pluginVersion}".toString())
assertContains(log, 'This plugin has 5 goals')
['ex:add', 'ex:help', 'ex:init', 'ex:setup', 'ex:uninstall'].each { assertContains(log, it) }
assertContains(log, 'Add a dependency to the project')
assertContains(log, 'Register the ex plugin prefix in your Maven settings')

// Detailed help documents each parameter with its ex.* user property.
mvn(basedir, localRepositoryPath, ['-B', '-o', "com.sebas3261:ex-maven-plugin:${pluginVersion}:help".toString(),
        '-Ddetail', '-Dgoal=add'], null, [:])
        .assertSuccess().assertOutput('User property: ex.deps').assertOutput('User property: ex.scope')
        .assertOutput('mvn ex:add -Dex.deps=junit-jupiter -Dex.scope=test')
return true
