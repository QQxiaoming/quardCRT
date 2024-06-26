<div style="text-align: right"><a href="../../en/latest/faq.html">🇺🇸 English</a> | <a href="../../zh-cn/latest/faq.html">🇨🇳 简体中文</a> | <a href="../../zh-tw/latest/faq.html">🇭🇰 繁體中文</a> | <a href="../../ja/latest/faq.html">🇯🇵 日本語</a></div>

# FAQ

- Is quardCRT free software?
    - Yes, quardCRT is open source software, following the GPLv3 license. You can get the full compilable source code on [GitHub](https://github.com/QQxiaoming/quardCRT)/[Gitee](https://gitee.com/QQxiaoming/quardCRT).

- Can quardCRT software be used for commercial purposes?

    - quardCRT follows the GPLv3 open source license. You can freely use the quardCRT software for commercial purposes under the premise of following the GPLv3 license (but please read the relevant terms carefully). You can read the license agreement [here](./license.md).

- Why did quardCRT software choose the GPLv3 license instead of other open source licenses?
    - quardCRT software itself relies on many other open source software. In order to respect the license agreements of other open source software and comply with the terms of the agreements, quardCRT chose the GPLv3 license.

- Is quardCRT software secure? How are privacy information such as passwords stored?
    - quardCRT software is open source software, and you can view the source code to understand how the software works. quardCRT software does not store key privacy information such as passwords to disk.
    - The principle of saving your session information in quardCRT software is: store non-password information (such as host name, port number, username, etc.) in the configuration file. You can read the [configuration](./configuration.md) page to learn about the configuration file path. Password information relies on the open source software [QtKeychain](https://github.com/frankosterfeld/qtkeychain) to provide system keychain services, submit password information to the system keychain service, and the system keychain service uniformly saves password information. The security of the password depends on the security of the system keychain service. In general, Windows, macOS, and Linux systems supported by the official provide secure keychain services. The system keychain service usually requires you to enter a password to unlock when logging into the system (Windows) or verify the login password or fingerprint each time you access (macOS). However, please note that the security of the password depends on the system keychain service, and quardCRT software itself is not responsible for the security of the password.
    - If you still have doubts about the security of the password, you can choose not to save the password and enter the password each time you connect.
