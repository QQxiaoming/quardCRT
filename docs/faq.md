<div style="text-align: right"><a href="../../en/latest/faq.html">🇺🇸 English</a> | <a href="../../zh-cn/latest/faq.html">🇨🇳 简体中文</a> | <a href="../../zh-tw/latest/faq.html">🇭🇰 繁體中文</a> | <a href="../../ja/latest/faq.html">🇯🇵 日本語</a></div>

# FAQ

This page answers the most common questions about licensing, source code availability, and how quardCRT stores sensitive data.

## Is quardCRT free and open source?

Yes. quardCRT is open source software released under the GPLv3 license.

You can get the full source code from:

- [GitHub](https://github.com/QQxiaoming/quardCRT)
- [Gitee](https://gitee.com/QQxiaoming/quardCRT)

For the full license text, see the [license page](./license.md).

## Can quardCRT be used commercially?

Yes, quardCRT can be used in commercial environments as long as your use complies with the GPLv3 license terms.

If you plan to redistribute quardCRT, modify it, or bundle it with other software, read the GPLv3 requirements carefully before doing so.

## Why does quardCRT use GPLv3?

quardCRT depends on and incorporates multiple open source components. GPLv3 was chosen to remain compatible with the licensing obligations of the software it builds on and to respect the license terms of upstream projects.

## Is quardCRT secure?

quardCRT is open source, which means you can inspect the code and verify how it behaves. Like any terminal or remote access tool, its overall security depends on:

- the security of the host operating system
- the security of the remote systems you connect to
- the safety of the plugins and scripts you choose to use
- the way you handle credentials and local access on your machine

quardCRT itself does not store passwords as plain text in the normal configuration file.

## Where are settings stored?

Regular application and session metadata such as host name, port, and user name are stored in the quardCRT configuration file.

You can confirm the active configuration file path from `Options > Global Options > Advanced`. The [configuration page](./configuration.md) describes this in more detail.

## How are passwords stored?

Saved passwords are handled separately from the main configuration file.

- Non-secret data such as host name, port, and user name are stored in the quardCRT settings file.
- Passwords and similar secret values rely on [QtKeychain](https://github.com/frankosterfeld/qtkeychain), which integrates with the operating system's keychain or credential storage service.

In other words, quardCRT delegates password storage to the platform keychain instead of writing passwords directly into the normal configuration file.

## What does that mean on each platform?

The exact behavior depends on the operating system and desktop environment:

- On Windows, access is generally tied to the logged-in user account.
- On macOS, the system may ask for your login password or biometric confirmation when protected secrets are accessed.
- On Linux, behavior depends on the available keychain service provided by the desktop environment or system setup.

Because of this, the protection level of saved passwords depends partly on the security of your system keychain service and local machine.

## Can I avoid saving passwords?

Yes. If you do not want passwords to be stored through the system keychain, simply do not save them in your session configuration. You can enter the password manually each time you connect.

## Does quardCRT store all private data on disk?

No. The main distinction is:

- regular session metadata goes into the configuration file
- secret values are intended to be stored through the system keychain

If you are auditing a deployment, review both the quardCRT configuration file and your platform keychain configuration.
