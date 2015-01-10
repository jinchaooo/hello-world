if [ $# -eq 1 -a "$1" == "c" ]
then
  ls | grep -v -e cc -e Makefile.am -e build.sh | xargs rm -fr
  exit 0
fi

autoscan
mv configure.scan configure.ac
sed -i '/AC_INIT/a AM_INIT_AUTOMAKE([foreign])' configure.ac
aclocal
autoconf
autoheader
automake --add-missing
./configure
