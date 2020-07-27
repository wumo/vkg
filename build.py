from cpt.packager import ConanMultiPackager
from cpt.tools import get_bool_from_env

if __name__ == "__main__":
    builder = ConanMultiPackager()
    need_build = get_bool_from_env("CONAN_NEED_BUILD")
    
    if need_build:
        builder.add()
        # builder.add_common_builds() #uncomment to allow more builds
        builder.run()
    else:
        if not builder.upload_only_when_tag or builder.ci_manager.is_tag():
            if not builder.skip_check_credentials:
                builder.remotes_manager.add_remotes_to_conan()
                builder.auth_manager.login(builder.remotes_manager.upload_remote_name)
            name, version, user, channel, _ = builder.reference
            builder.conan_api.export(".", name, version, user, channel)
            builder.uploader.upload_recipe(builder.reference, True)
