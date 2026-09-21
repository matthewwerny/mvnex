import static com.sebas3261.ex.it.ItSupport.*
import static com.sebas3261.ex.it.WireMockSupport.*

def fq = { goal -> "com.sebas3261:ex-maven-plugin:${pluginVersion}:${goal}".toString() }

def settings = new File(basedir, 'settings.xml')
mvn(basedir, localRepositoryPath, ['-o', '-s', settings.absolutePath, fq('setup')], null, [:]).assertSuccess()
assertBytes(new File(basedir, 'expected-settings.xml').bytes, settings)
mvn(basedir, localRepositoryPath, ['-o', '-s', settings.absolutePath, 'ex:help'], null, [:])
        .assertSuccess().assertOutput('This plugin has 5 goals')
return true
