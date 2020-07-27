import os, sys, platform
from os.path import join, abspath
import pathlib

_, assetsDir, binAssetsDir = sys.argv
try:
    binDir = abspath(join(binAssetsDir, os.pardir))
    pathlib.Path(assetsDir).mkdir(parents=True, exist_ok=True)
    pathlib.Path(binDir).mkdir(parents=True, exist_ok=True)
    if platform.system() == 'Windows':
        import _winapi
        
        _winapi.CreateJunction(assetsDir, binAssetsDir)
    else:
        os.symlink(assetsDir, binAssetsDir)
    print(f"Created Symlink {binAssetsDir} -> {assetsDir}")
except FileExistsError:
    print(f"Symlink already exists: {binAssetsDir}")
except Exception as e:
    print(f"{sys.argv}\n{e}")
