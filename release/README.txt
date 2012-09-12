#
# release distribution version control README.txt
#

I.  Relesase Version

    A distribution should have fields: name, major version, minor version,
    and build number. That is: name_major_minor_build.extension.
    The underbar '_' is reserved as a field separator.
    The fields are explained below.

    Simple example: propeller-gcc_v0_1_252.tar.gz

    A. Distribution name

        A distribution name should describe the program and contain
        letters, numbers, or the dash '-' symbol.

    B. Major and minor versions

        A major version number greater than 0 is given to a release
        for general availability. A minor version is used for significant
        updates to a major release.

        The major version should be preceded by 'v'.
        Major and minor numbers are positive and can be more than one digit.

    C. Build numbers

        A build number is based on a periodic build. Builds can be nightly.
        Each time the release script completes successfully, the
        build number increments and a release package is made.


II. Files used in versioning.

    A. Version files

        1.  VERSION.txt defines the current version.

            The VERSION.txt can contain comments with '#' in the first
            column of each row. Only one line not containing '#' will be
            interpreted as the version line.

            The version will take the form name_vM_m_.

        2.  name_vM_m.txt is one or more files having build pointer information.

            The build numbers and associated timestamps are kept in
            the vM_m.txt file. The last build number in the file is always
            the most recent. The first build number in the file is always 0.

            Build numbers are always kept as: number date.
            Example: 126 2011-10-6-19:45:08

            Timestamps for build numbers can be collected with:
            $ date '+%Y-%m-%d-%H:%M:%S'

    B. Procedure

        1.  Script release.sh is used to create build information.

            Each time a build completes successfully, release.sh runs,
            the build number will increment, and the date is collected
            and put into the current version file defined in VERSION.txt.

        2.  After running release.sh, all changed version files
            should be committed to the repository.

            Never commit any tar or zip files.


