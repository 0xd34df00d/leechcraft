// Job shells of the LeechCraft CI, applied by the `ci-seed` job on every push. The build logic lives in the
// Jenkinsfile next to this file; a job only says which profile it runs, when, what it triggers and what it keeps.

import groovy.transform.Field

@Field final String REPO = 'https://github.com/0xd34df00d/leechcraft.git'
@Field final String PROJECT_URL = 'https://github.com/0xd34df00d/leechcraft/'
@Field final String NOTIFY = '0xd34df00d@gmail.com'
// Job DSL cannot change a job's type, so the quick job stays out of its hands until the `leechcraft`
// directory has been converted from the matrix job to a pipeline job.
@Field final boolean QUICK_JOB_IS_PIPELINE = false

def ciJob(Map opts = [:], String name, String profile, String desc) {
    pipelineJob(name) {
        description desc
        if (opts.keep)
            logRotator { numToKeep opts.keep }
        properties {
            githubProjectUrl PROJECT_URL
            disableConcurrentBuilds()
            if (opts.onPush || opts.cron) {
                pipelineTriggers {
                    triggers {
                        if (opts.onPush)
                            githubPush()
                        if (opts.cron)
                            cron opts.cron
                    }
                }
            }
        }
        parameters {
            stringParam 'PROFILE', profile, 'quick: the primary distro only; full: every distro under tools/docker/base'
            stringParam 'SKIP_CELLS', opts.skip ?: '', 'Comma-separated cells to leave out, e.g. debian_experimental-clang'
            stringParam 'DOWNSTREAM', opts.downstream ?: '', 'Job to trigger after a green build; empty for none'
            stringParam 'NOTIFY', NOTIFY, 'Mail recipients on failures and on status changes; empty disables mail'
        }
        definition {
            cpsScm {
                scm {
                    git {
                        remote { url REPO }
                        branch '*/master'
                    }
                }
                scriptPath 'tools/ci/Jenkinsfile'
                lightweight true
            }
        }
    }
}

ciJob('leechcraft-full', 'full', 'Every supported distro, one cell at a time, after each green build of the primary distro.')

if (QUICK_JOB_IS_PIPELINE)
    ciJob('leechcraft', 'quick', 'Modular live environment. The primary distro, on every push.',
          onPush: true, downstream: 'leechcraft-full')

freeStyleJob('ci-seed') {
    description 'Applies tools/ci/jobs.groovy, the versioned definitions of the CI jobs.'
    label 'real'
    logRotator { numToKeep 30 }
    scm {
        git {
            remote { url REPO }
            branch '*/master'
        }
    }
    triggers { githubPush() }
    steps {
        jobDsl {
            targets 'tools/ci/jobs.groovy'
            removedJobAction 'IGNORE'
            removedViewAction 'IGNORE'
            lookupStrategy 'JENKINS_ROOT'
            sandbox false
        }
    }
}
