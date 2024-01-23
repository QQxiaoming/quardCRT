
UNAMEOUT="$(uname -s)"
case "${UNAMEOUT}" in
    Linux*)    
        date_info=$(date +"%Y-%m-%dT%H:%M:%S.%3NZ")
        ;;
    Darwin*)    
        date_info=$(gdate +"%Y-%m-%dT%H:%M:%S.%3NZ")
        ;;
esac

git_info=$(git describe --always --long --dirty --abbrev=10 --tags)
git_short_hash=$(git rev-parse --short HEAD)
git_hash=$(git rev-parse HEAD)

echo "#ifndef __BUILD_INFO_H__"
echo "#define __BUILD_INFO_H__"
echo
echo "#define BUILD_INFO \"${git_info}\""
echo "#define BUILD_INFO_LEN ${#git_info}"
echo "#define BUILD_HASH \"${git_hash}\""
echo "#define BUILD_HASH_LEN ${#git_hash}"
echo "#define BUILD_SHORT_HASH \"${git_short_hash}\""
echo "#define BUILD_SHORT_HASH_LEN ${#git_short_hash}"
echo "#define BUILD_DATE \"${date_info}\""
echo "#define BUILD_DATE_LEN ${#date_info}"
echo
echo "#endif /* __BUILD_INFO_H__ */"
