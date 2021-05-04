message () {
    REV_FILE="filesystem/usr/etc/revision"
    REV_FILE_OLD="$REV_FILE.old"
    local msg
    local first_line

    mv $REV_FILE $REV_FILE_OLD

    echo -n "Enter revision message: "
    read msg
    if [ ! -z "$msg" -a "$msg" != " " ]; then
        echo "[$(date)] $msg" > $REV_FILE
        head -n 4 $REV_FILE_OLD >> $REV_FILE
    else
        first_line=$(head -n 1 $REV_FILE_OLD)
        msg=$(awk -F] '{print $2}' <<< $first_line)
        msg=$(echo $msg | sed 's/ *$//g')
        echo "[$(date)] $msg" > $REV_FILE
        head -n 5 $REV_FILE_OLD | tail -4 >> $REV_FILE
    fi
}

alias mk010='fakeroot make PROFILE=en7521_G010_VNPTT_demo'
alias mk010_oversea='fakeroot make PROFILE=en7521_G010_OVERSEA'
alias mk040='fakeroot make PROFILE=en7521_7615D_VNPTT_demo'
alias mk020='fakeroot make PROFILE=en7521_7615D_G020_VNPTT'
alias mkd010="message; mk010"
alias mkd020="message; mk020"
alias mkd040="message; mk040"
