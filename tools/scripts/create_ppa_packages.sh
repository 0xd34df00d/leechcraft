build_for_distr(){
	DIST=$1 #lucid
	DIST_BRANCH=$2 #second parameter

	cd leechcraft_*\~ppa1
	git pull > /dev/null
	DESC=`git describe`
	cd ..
	mv leechcraft_*\~ppa1 leechcraft_$DESC-$DIST\~ppa1  
	cd leechcraft_$DESC-$DIST\~ppa1
	cd debian
	git checkout $DIST_BRANCH

	mv changelog changelog.old
	{
	echo "leechcraft ($DESC-$DIST~ppa1) $DIST; urgency=low"
	echo
	echo '  * New upstream release.'
	echo 
	echo " -- Georg Rudoy <0xd34df00d@gmail.com>  `date -R`"
	echo
	} >changelog
	cat changelog.old >> changelog
	rm changelog.old

	git commit -a -m "New upstream release"
	cd ..
	debuild -S -sa
	cd ..
	dput ppa leechcraft_$DESC-$DIST\~ppa1_source.changes
}

build_for_distr lucid ubuntu
build_for_distr maverick ubuntu_10.10
