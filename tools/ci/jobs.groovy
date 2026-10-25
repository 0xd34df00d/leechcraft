// Job shells of the LeechCraft CI, applied by the `ci-seed` job on every push. The build logic lives in the
// Jenkinsfile and images.Jenkinsfile next to this file; a job only says which script and profile it runs, when,
// what it triggers and what it keeps.

import groovy.transform.Field

@Field final String REPO = 'https://github.com/0xd34df00d/leechcraft.git'
@Field final String PROJECT_URL = 'https://github.com/0xd34df00d/leechcraft/'
@Field final String NOTIFY = '0xd34df00d@gmail.com'
@Field final String IMAGES_JOB = 'leechcraft-images'
// Username and password for the registry the images are pushed to, as stored on the controller.
@Field final String REGISTRY_CREDENTIALS = '0f9a59fe-7663-417a-a6d5-9ead2e1b776f'

def lcPipeline(Map opts = [:], String name, String script, String desc, Closure jobParameters) {
    pipelineJob(name) {
        description desc
        if (opts.keep)
            logRotator {
                numToKeep opts.keep
                if (opts.keepArtifacts)
                    artifactNumToKeep opts.keepArtifacts
            }
        properties {
            githubProjectUrl PROJECT_URL
            disableConcurrentBuilds()
            if (opts.onPush || opts.cron) {
                pipelineTriggers {
                    triggers {
                        if (opts.onPush)
                            githubPush()
                        // in this generated DSL a trigger with arguments takes a block, unlike githubPush()
                        if (opts.cron)
                            cron { spec opts.cron }
                    }
                }
            }
        }
        parameters jobParameters
        definition {
            cpsScm {
                scm {
                    git {
                        remote { url REPO }
                        branch '*/master'
                    }
                }
                scriptPath script
                lightweight true
            }
        }
    }
}

def ciJob(Map opts = [:], String name, String profile, String desc) {
    lcPipeline(opts, name, 'tools/ci/Jenkinsfile', desc) {
        stringParam 'PROFILE', profile, 'quick: the primary distro only; full: every distro under tools/docker/base'
        stringParam 'SKIP_CELLS', opts.skip ?: '', 'Comma-separated cells to leave out, e.g. debian_experimental-clang'
        stringParam 'DOWNSTREAM', opts.downstream ?: '', 'Job to trigger after a green build; empty for none'
        stringParam 'IMAGES', opts.images ?: '',
                    'Job that rebuilds the images; triggered instead of DOWNSTREAM when the push touched them. Empty for none'
        stringParam 'NOTIFY', NOTIFY, 'Mail recipients on failures and on status changes; empty disables mail'
    }
}

ciJob('leechcraft-full', 'full', 'Every supported distro, one cell at a time, after each green build of the primary distro.')

ciJob('leechcraft', 'quick', 'Modular live environment. The primary distro, on every push.',
      onPush: true, downstream: 'leechcraft-full', images: IMAGES_JOB)

// Daily at four in the morning, US Central time, and after a push that touches the Dockerfiles or the packaging (the
// quick job sends those here).
lcPipeline(IMAGES_JOB, 'tools/ci/images.Jenkinsfile',
           'The CI images: rebuilt from scratch daily and after a push that touches them; a change triggers the full build.',
           cron: 'TZ=America/Chicago\nH 4 * * *', keep: 60, keepArtifacts: 7) {
    stringParam 'DISTROS', '', 'Comma-separated distros to rebuild; empty for every one under tools/docker/base'
    booleanParam 'PUSH', true, 'Push the images whose packages changed to the registry'
    stringParam 'DOWNSTREAM', 'leechcraft-full', 'Job to trigger when an image changed; empty for none'
    booleanParam 'ALWAYS_TRIGGER', false, 'Trigger DOWNSTREAM even when no image changed'
    stringParam 'REGISTRY_CREDENTIALS', REGISTRY_CREDENTIALS, 'Username/password credentials for the registry'
    stringParam 'NOTIFY', NOTIFY, 'Mail recipients on status changes; empty disables mail'
}

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
