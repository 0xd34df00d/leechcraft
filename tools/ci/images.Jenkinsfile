// Rebuilds the CI images: every distro under tools/docker/base, or those in DISTROS, from scratch and from a freshly
// pulled base image, through tools/docker/buildimage.sh. The distros build concurrently under one build lock: the lock
// keeps compiles off the machine while images build, not the image builds off each other, and so is held for the
// longest image build rather than for all of them in turn. A distro whose build fails keeps its previous image and
// fails the run; the others are still rebuilt. Each new image's package list is compared with the previous image's:
// the diff is in the log and the lists are archived, which is where to look when a cell breaks after a rebuild. An image whose
// packages changed is pushed to the registry (PUSH), and when any changed, or ALWAYS_TRIGGER says so, DOWNSTREAM runs
// against the new set. Mail goes to NOTIFY on a change of status only: an image that stays broken is one piece of news.

import groovy.transform.Field

@Field final String AGENT = 'real'
@Field final String BUILD_LOCK = 'lc-build'
@Field final String IMAGE_PREFIX = 'leechcraft/ci_'
@Field final String STATE = 'images-state'
// per distro: changed | unchanged | unknown | failed, and the compiler versions, as buildimage.sh reports them
@Field final Map<String, String> status = [:]
@Field final Map<String, String> compilers = [:]

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

void push(String distro) {
    withCredentials([usernamePassword(credentialsId: params.REGISTRY_CREDENTIALS,
                                      usernameVariable: 'REGISTRY_USER', passwordVariable: 'REGISTRY_PASSWORD')]) {
        // no -x: the login line must not be echoed, even though the binding masks the secret itself
        sh(label: "push ${distro}", script: "#!/bin/sh -e\n" +
            "echo \"\$REGISTRY_PASSWORD\" | docker login -u \"\$REGISTRY_USER\" --password-stdin\n" +
            "docker push ${IMAGE_PREFIX}${distro}:latest\n")
    }
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
        compilers[distro] = readFile("${STATE}/${distro}.compilers").trim().replace('\n', ', ')
        if (status[distro] == 'unknown')
            unstable("${distro}: the image was built, but its packages could not be listed, so it counts as changed; " +
                     'a package manager buildimage.sh does not know?')
        else if (rc == 3)
            unstable("${distro}: the image was built, but listing its compilers or exporting its packages failed")
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
            stage('Build') {
                lock(BUILD_LOCK) {
                    parallel distros.collectEntries { String distro -> [(distro): { buildImage(distro) }] }
                }
            }
            List<String> moved = distros.findAll { status[it] in ['changed', 'unknown'] }
            if (params.PUSH && moved) {
                stage('Push') {
                    for (String distro in moved)
                        push(distro)
                    sh(label: 'docker logout', script: 'docker logout || true')
                }
            }
            // the debug symbol packages are exported with the rest but not kept: 50 MB per image per day, for a repository
            // that does not exist yet
            archiveArtifacts(artifacts: "${STATE}/**", excludes: "${STATE}/packages/*/debug/**", allowEmptyArchive: true)
            // after the exports: the pruning drops what the rebuilds left untagged and the cache even a --no-cache build writes
            sh(label: 'prune', script: 'docker image prune -f && docker builder prune -f')
            currentBuild.description = distros.collect { "${it}: ${status[it]}" + (compilers[it] ? " (${compilers[it]})" : '') }.join('; ')
            if (params.DOWNSTREAM && (moved || params.ALWAYS_TRIGGER)) {
                stage("Trigger ${params.DOWNSTREAM}") {
                    catchError(buildResult: 'SUCCESS', stageResult: 'FAILURE') {
                        build job: params.DOWNSTREAM, wait: false
                    }
                }
            }
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
