rm -f	Makefile
rm -f	Makefile.in
rm -f	aclocal.m4
rm -f -r autom4te.cache/
rm -f	compile
rm -f	config.h
rm -f	config.h.in*
rm -f	config.log
rm -f	config.status
rm -f	configure
rm -f	depcomp
rm -f -r ghpsdr3-*.tar.gz
rm -f	install-sh
rm -f	missing
rm -f	stamp-h1
rm -f	config.guess
rm -f	config.sub
rm -f	ghpsdr*tar.gz


r () 
{
       cd "$1"
#       echo "entering in $1"
       if [ -x cleanup.sh ]; then
          echo executing local cleanup....
          bash -c ./cleanup.sh;
       fi;


    for d in *
    do
        if [ -d "$d" ]; then
            echo "DIR: $d"
            ( r "$d" )
        fi;
    done

}


r ./trunk/


