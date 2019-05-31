from rever.activity import dockeractivity

$GITHUB_ORG = 'cyclus'
$PROJECT = $GITHUB_REPO = 'cycamore'

$CHANGELOG_FILENAME = 'CHANGELOG.rst'
$CHANGELOG_TEMPLATE = 'TEMPLATE.rst'

$DOCKER_APT_DEPS = ['libc6', 'libc6-i386', 'libc6-dev', 'libc-dev', 'gcc']
$DOCKER_CONDA_DEPS = ['cyclus', 'make', 'cmake', 'pkg-config', 'jinja2',
                      'gcc', 'nose', 'pytables']
$DOCKER_INSTALL_COMMAND = (
    'git clean -fdx && '
    './install.py --build_type=Release '
    '             -DBLAS_LIBRARIES="-L/opt/conda/lib -lopenblas" '
    '             -DLAPACK_LIBRARIES="-L/opt/conda/lib -lopenblas"'
    )


with! dockeractivity(name='cycamore-tests', lang='sh'):
    set -e
    set -x
    export CYCLUS_PATH=${HOME}/.local/lib/cyclus:/opt/conda/lib/cyclus
    export PATH=${HOME}/.local/bin:${PATH}
    cd tests
    cycamore_unit_tests
    nosetests

$ACTIVITIES = ['cycamore-tests', 'changelog', 'tag',
               'push_tag', 'conda_forge', 'ghrelease',
               ]
