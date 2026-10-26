// Rebuilds the CI images: every distro under tools/docker/base, or those in DISTROS, from scratch and from a freshly
// pulled base image, through tools/docker/buildimage.sh. The distros build concurrently under one build lock: the lock
// keeps compiles off the machine while images build, not the image builds off each other, and so is held for the
// longest image build rather than for all of them in turn. A distro whose build fails keeps its previous image and
// fails the run; the others are still rebuilt. Each new image's package list is compared with the previous image's:
// the diff is in the log and the lists are archived, which is where to look when a cell breaks after a rebuild. An image whose
// packages changed is pushed to the registry (PUSH) as soon as it is built, while the others still build: a push is a
// minute of single-core compression per image, so pushing them in a row after the last build cost as much again as the
// builds. When any changed, or ALWAYS_TRIGGER says so, DOWNSTREAM runs against the new set. Mail goes to NOTIFY on a
// change of status only: an image that stays broken is one piece of news.

import groovy.transform.Field

@Field final String AGENT = 'real'
@Field final String BUILD_LOCK = 'lc-build'
@Field final String IMAGE_PREFIX = 'leechcraft/ci_'
@Field final String STATE = 'images-state'
// the registry login of this run lives here, not in the agent user's docker config
@Field final String DOCKER_CONFIG = 'docker-config'
// per distro: changed | unchanged | unknown | failed, and the compiler versions, as buildimage.sh reports them
@Field final Map<String, String> status = [:]
@Field final Map<String, String> toolchain = [:]

List<String> pickDistros() {
    List<String> all = sh(returnStdout: true, script: 'ls tools/docker/base').trim().tokenize('\n')
    List<String> wanted = (params.DISTROS ?: '').tokenize(', ')
    if (!wanted)
        return all
    List<String> unknown = wanted - all
    if (unknown)
        error("no such distro: ${unknown.join(', ')}; there is ${all.join(', ')}")
    all.findAll { it in wanted }
}

String dockerConfig() {
    "${env.WORKSPACE}/${DOCKER_CONFIG}"
}

// One login for the whole run: the branches push as soon as their image is built, and a login and logout per branch
// would race the others.
void login() {
    withCredentials([usernamePassword(credentialsId: params.REGISTRY_CREDENTIALS,
                                      usernameVariable: 'REGISTRY_USER', passwordVariable: 'REGISTRY_PASSWORD')]) {
        // no -x: the login line must not be echoed, even though the binding masks the secret itself
        sh(label: 'docker login', script: "#!/bin/sh -e\n" +
            "mkdir -p ${dockerConfig()}\n" +
            "echo \"\$REGISTRY_PASSWORD\" | DOCKER_CONFIG=${dockerConfig()} docker login -u \"\$REGISTRY_USER\" --password-stdin\n")
    }
}

void logout() {
    sh(label: 'docker logout', script: "DOCKER_CONFIG=${dockerConfig()} docker logout || true; rm -rf ${dockerConfig()}")
}

void push(String distro) {
    sh(label: "push ${distro}", script: "DOCKER_CONFIG=${dockerConfig()} docker push ${IMAGE_PREFIX}${distro}:latest")
}

void buildImage(String distro) {
    status[distro] = 'failed'
    catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') {
        int rc = sh(returnStatus: true, label: "build ${distro}", script: "tools/docker/buildimage.sh ${distro} ${STATE}")
        if (rc == 1)
            error("${distro}: the image build failed; the previous image stays")
        if (rc != 0 && rc != 3)
            error("${distro}: buildimage.sh failed with status ${rc}")
        String statusFile = "${STATE}/${distro}.status"
        if (!fileExists(statusFile))
            error("${distro}: buildimage.sh exited with ${rc} but left no ${statusFile}")
        status[distro] = readFile(statusFile).trim()
        toolchain[distro] = readFile("${STATE}/${distro}.toolchain").trim().replace('\n', ', ')
        if (status[distro] == 'unknown')
            unstable("${distro}: the image was built, but its packages could not be listed, so it counts as changed; " +
                     'a package manager buildimage.sh does not know?')
        else if (rc == 3)
            unstable("${distro}: the image was built, but listing its toolchain versions or exporting its packages failed")
        if (params.PUSH && status[distro] in ['changed', 'unknown'])
            push(distro)
    }
}

void notifyIfNeeded() {
    String now = currentBuild.currentResult
    String previous = currentBuild.previousCompletedBuild?.result ?: 'SUCCESS'
    if (!params.NOTIFY || now == 'ABORTED' || now == previous)
        return
    emailext(to: params.NOTIFY, recipientProviders: [culprits(), requestor()],
             subject: '$PROJECT_NAME - Build #$BUILD_NUMBER - $BUILD_STATUS',
             body: '$PROJECT_NAME - Build #$BUILD_NUMBER - $BUILD_STATUS\n\n$BUILD_URL\n\nChanges:\n$CHANGES')
}

timeout(time: 4, unit: 'HOURS') {
    try {
        node(AGENT) {
            checkout scm
            List<String> distros = pickDistros()
            sh "rm -rf ${STATE} && mkdir ${STATE}"
            if (params.PUSH)
                login()
            try {
                stage('Build and push') {
                    lock(BUILD_LOCK) {
                        parallel distros.collectEntries { String distro -> [(distro): { buildImage(distro) }] }
                    }
                }
            } finally {
                if (params.PUSH)
                    logout()
            }
            List<String> moved = distros.findAll { status[it] in ['changed', 'unknown'] }
            // the downstream build uses the local images, so it need not wait for the bookkeeping below
            if (params.DOWNSTREAM && (moved || params.ALWAYS_TRIGGER)) {
                stage("Trigger ${params.DOWNSTREAM}") {
                    catchError(buildResult: 'SUCCESS', stageResult: 'FAILURE') {
                        build job: params.DOWNSTREAM, wait: false
                    }
                }
            }
            // the debug symbol packages are exported with the rest but not kept: 50 MB per image per day, for a repository
            // that does not exist yet
            archiveArtifacts(artifacts: "${STATE}/**", excludes: "${STATE}/packages/*/debug/**", allowEmptyArchive: true)
            // after the exports: the pruning drops what the rebuilds left untagged and the cache even a --no-cache build writes
            sh(label: 'prune', script: 'docker image prune -f && docker builder prune -f')
            currentBuild.description = distros.collect { "${it}: ${status[it]}" + (toolchain[it] ? " (${toolchain[it]})" : '') }.join('; ')
        }
    } catch (org.jenkinsci.plugins.workflow.steps.FlowInterruptedException e) {
        currentBuild.result = 'ABORTED'
        throw e
    } catch (e) {
        currentBuild.result = 'FAILURE'
        throw e
    } finally {
        notifyIfNeeded()
    }
}
