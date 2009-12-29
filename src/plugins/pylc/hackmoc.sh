cat $1 | sed 's/WrapperObject::qt_metacast\>/WrapperObject::qt_metacast_dummy/g' > $1.new
mv $1.new $1
