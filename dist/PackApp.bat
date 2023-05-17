
SET AP_ROOT=\AP
SET AP_OUT_ROOT=\OUT
SET SD_AP_OUT_ROOT=\vxupdate\OUT
SET SD_ROOT=\vxupdate
SET DL_VERSION=CTBC002


if [ -x $AP_ROOT ]
then
  echo "$TEMPDIR EXIST"
else
  echo "$TEMPDIR NOT EXIST"
  mkdir $TEMPDIR
fi

cd $TEMPDIR
@cd %AP_ROOT%\
@copy CA.CAP ..\%SD_ROOT%\%GEN_USB%\1
@copy CA.mci %DL_ROOT%\%DL_VERSION%\%GEN_USB%\1


@cd %AP_ROOT%\%DL_VERSION%\%GEN_GROUP%
@copy setgroup.1 %DL_ROOT%\%DL_VERSION%\%GEN_USB%\1

@cd %DL_ROOT%\%DL_VERSION%\%GEN_USB%
@echo y | del *.*

@cd %DL_ROOT%\%DL_VERSION%\%GEN_APPL%
7z a -tzip %DL_ROOT%\%DL_VERSION%\%DL_APPL%\NCCCAPPL.zip