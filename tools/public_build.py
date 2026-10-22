# Normalize compiler source paths in public firmware; no local account paths in bin.
from pathlib import Path
Import('env')
env.Append(CCFLAGS=['-ffile-prefix-map=' + str(Path.home()) + '=/build/home',
                    '-ffile-prefix-map=' + env.subst('$PROJECT_DIR') + '=/build/project'])
