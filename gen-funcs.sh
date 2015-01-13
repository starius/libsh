#!/bin/bash

set -e

[ $# != 3 ] && echo "Usage: ${0##*/} HEADER SOURCE CMAKE" >&2 && exit 1

HEADER_FILE="$1"
SOURCE_FILE="$2"
CMAKE_FILE="$3"

exec 3> "$HEADER_FILE"
exec 4> "$SOURCE_FILE"
exec 5> "$CMAKE_FILE"

printf "// Generated by gen-funcs.sh\\n" >&3
printf "\\n" >&3
printf "#ifndef _SH_FUNCS_H\\n" >&3
printf "#define _SH_FUNCS_H\\n" >&3
printf "\\n" >&3
printf "#ifdef __cplusplus\\n" >&3
printf "extern \"C\"\\n" >&3
printf "{\\n" >&3
printf "#endif\\n" >&3
printf "\\n" >&3
printf "#include \"ex.h\"\\n" >&3
printf "\\n" >&3

printf "// Generated by gen-funcs.sh\\n" >&4
printf "\\n" >&4
printf "#define _POSIX_C_SOURCE 200809L\\n" >&4 # Because I want to get all POSIX functions
printf "\\n" >&4
printf "#include \"funcs.h\"\\n" >&4
printf "\\n" >&4

printf "# Generated by gen-funcs.sh\\n" >&5
printf "\\n" >&5

sed 's/#.*$//' | while read -r LINE; do
	[ -z "$LINE" ] && continue

	IFS="|" read -r TYPES IMPL DECLARATION ON_ERROR RESULT MESSAGE <<< "$LINE"

	read -r TYPES <<< "$TYPES"
	read -r IMPL <<< "$IMPL"
	read -r DECLARATION <<< "$DECLARATION"
	read -r ON_ERROR <<< "$ON_ERROR"
	read -r RESULT <<< "$RESULT"
	read -r MESSAGE <<< "$MESSAGE"

	if [ "$ON_ERROR" = external ]; then
		if [ "$RESULT" != external ] || [ "$MESSAGE" != external ]; then
			echo "${0##*/}: mixed external and non-external" >&2
			exit 1
		fi
	else
		if [ "$RESULT" = external ] || [ "$MESSAGE" = external ]; then
			echo "${0##*/}: mixed external and non-external" >&2
			exit 1
		fi
	fi

	if [ "$ON_ERROR" = custom ]; then
		if [ "$MESSAGE" != custom ]; then
			echo "${0##*/}: mixed custom and non-custom, line:" >&2
			printf '%s\n' "$LINE" >&2
			exit 1
		fi
	else
		if [ "$MESSAGE" = custom ]; then
			echo "${0##*/}: mixed custom and non-custom, line:" >&2
			printf '%s\n' "$LINE" >&2
			exit 1
		fi
	fi

	if [ "$ON_ERROR" != external ] && [ "$ON_ERROR" != custom ] && grep '\.\.\.' <<< "$DECLARATION" > /dev/null; then
		echo "${0##*/}: not external and not custom and '...'" >&2
		exit 1
	fi

	DECLARATION="$(sed 's/^\([^(]*[^(0-9A-Z_a-z]\)\([0-9A-Z_a-z][0-9A-Z_a-z]*\) *(\(.*\)) *$/\1@\2@\3/' <<< "$DECLARATION")"

	if ! grep "@" <<< "$DECLARATION" > /dev/null; then
		echo "${0##*/}: cannot parse, line:" >&2
		printf '%s\n' "$LINE" >&2
		exit 1
	fi

	IFS="@" read -r TYPE NAME ARGS <<< "$DECLARATION"

	[ "$ON_ERROR" != external ] && [ "$ARGS" = "" ] && echo "${0##*/}: ARGS is empty" >&2 && exit 1

	if [ "$ON_ERROR" != external ] && [ "$ON_ERROR" != custom ]; then
		if [ "$ARGS" = "void" ]; then
			SHORT_ARGS=""
		else
			SHORT_ARGS="$(tr ',' '\n' <<< "$ARGS" | sed -e 's/\[[ 0-9]*]$//' -e 's/^.*[^0-9A-Z_a-z]\([0-9A-Z_a-z]*\)$/\1/' | tr '\n' ',')"
			SHORT_ARGS="$(sed -e 's/,$//' -e 's/,/, /g' <<< "$SHORT_ARGS")"
		fi
	fi

	if [ "$ON_ERROR" != external ] && [ "$ON_ERROR" != custom ]; then
		if grep '\* *$' <<< "$TYPE" > /dev/null; then
			if [ "$ON_ERROR" != NULL ]; then
				echo "${0##*/}: TYPE is pointer and ON_ERROR is not NULL, line:" >&2
				printf '%s\n' "$LINE" >&2
				exit 1
			fi
		else
			if [ "$ON_ERROR" = NULL ]; then
				echo "${0##*/}: TYPE is not pointer and ON_ERROR is NULL, line:" >&2
				printf '%s\n' "$LINE" >&2
				exit 1
			fi
		fi
	fi

	if [ "$ON_ERROR" != external ]; then
		case "$RESULT" in
			value)
				read -r RETURN_TYPE <<< "$TYPE"
				;;
			void)
				RETURN_TYPE="void"
				;;
			noreturn)
				RETURN_TYPE="SH_NORETURN void"
				;;
			*)
				echo "${0##*/}: $RESULT: wrong result, line:" >&2
				printf '%s\n' "$LINE" >&2
				exit 1
				;;
		esac
	fi

	includes(){
		tr ',' '\n' <<< "$1" | sed -e 's/^ *//' -e 's/ *$//' | grep -v '^$' | sed 's/^\(.*\)$/#include <\1>/'
	}

	[ -z "$IMPL" ] && CMAKE_HEADER="$TYPES" || CMAKE_HEADER="$IMPL"

	printf "check_symbol_exists ($NAME \"${CMAKE_HEADER%%,*}\" SH_HAVE_$NAME)\\n" >&5

	printf "#cmakedefine SH_HAVE_$NAME\\n" >&3
	printf "#ifdef SH_HAVE_$NAME\\n" >&3
	includes "$TYPES" >&3

	if [ "$ON_ERROR" != external ]; then
		printf "$RETURN_TYPE\\n" >&3
		printf "sh_x_$NAME ($ARGS);\\n" >&3
	fi

	printf "#endif\\n" >&3
	printf "\\n" >&3

	[ "$ON_ERROR" = external ] && continue

	printf "#ifdef SH_HAVE_$NAME\\n" >&4
	includes "$IMPL" >&4

	printf "$RETURN_TYPE\\n" >&4
	printf "sh_x_$NAME ($ARGS)\\n" >&4

	if [ "$ON_ERROR" = custom ]; then
		while :; do
			! IFS="" read -r CUSTOM && echo "${0##*/}: cannot read custom line" >&2 && exit 1

			[ "$CUSTOM" = "}" ] && break

			printf "%s\\n" "$CUSTOM" >&4
		done
	else
		if [ "$TYPE" = "void " ]; then
			echo "${0##*/}: type is void" >&2
			exit 1
		fi

		printf "{\\n" >&4
		printf "  ${TYPE}result = $NAME ($SHORT_ARGS);\\n" >&4

		case "$ON_ERROR" in
			"="*|"<"*|">"*|"!"*)
				printf "  if (result $ON_ERROR)\\n" >&4
				;;
			*)
				printf "  if (result == $ON_ERROR)\\n" >&4
				;;
		esac

		printf "    {\\n" >&4

		if [ -z "$MESSAGE" ]; then
			printf "      sh_throw (\"$NAME\");\\n" >&4
		else
			printf "      sh_throw (\"$NAME: \" %s);\\n" "$MESSAGE" >&4
		fi

		printf "    }\\n" >&4

		case "$RESULT" in
			value)
				printf "  return result;\\n" >&4
				;;
			void)
				;;
			noreturn)
				printf "  sh_throwx (\"$NAME: succeed, but returned\");\\n" >&4
				;;
		esac
	fi

	printf "}\\n" >&4
	printf "#endif\\n" >&4
	printf "\\n" >&4
done

printf "#ifdef __cplusplus\\n" >&3
printf "}\\n" >&3
printf "#endif\\n" >&3
printf "\\n" >&3
printf "#endif // ! _SH_FUNCS_H\\n" >&3