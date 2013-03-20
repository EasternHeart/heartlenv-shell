sed -i -e "s/$1/$2/g" $3
mv $3 `echo $3 | sed -e "s/$1/$2/g"`
