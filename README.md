-家mount- homefs
=============================

#概要
家をmountします。

![ロゴ](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/icon.jpg)




#何言っているんだこの人・・・

家をファイルシステムとしてmountします。

*Linux(fusefs)*

家を/mntにmountします。
```
./homefs 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX /mnt -o debug
```


*Windows(dokan)*

家をm:ドライブにmountします。
```
homefs.exe 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX m -o debug
```


#家をファイルシステム化するのです。
mountしたあとは、ls(windowsだとdir) や echo 、 cat(windowsだとtype)で家にアクセスできます。


```
mountして、
./homefs 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX /mnt/
```


ls で家電が表示されます、
```
ls -la /mnt/
drwxr-xr-x  5 root root    0  1月  1  1970 .
drwxr-xr-x 23 root root 4096 12月  8 15:39 ..
drwxr-xr-x  7 root root    0  1月  1  1970 エアコン
drwxr-xr-x  4 root root    0  1月  1  1970 照明
drwxr-xr-x  4 root root    0  1月  1  1970 扇風機
```


家電ディレクトリの中には、動作名があります。
```
ls -la /mnt/照明
drwxr-xr-x 4 root root 0  1月  1  1970 .
drwxr-xr-x 5 root root 0  1月  1  1970 ..
-rwxr--r-- 1 root root 9  1月  1  1970 .status
-rwxr--r-- 1 root root 0  1月  1  1970 けす
-rwxr--r-- 1 root root 1  1月  1  1970 つける
```


状態に1やonとかの文字列を書き込むと、その状態になります。
例えば、照明を消すには、このようにします。
```
echo 1 >/mnt/照明/けす
```


今の状態は、.statusを見ればわかります。
```
cat /mnt/照明/.status

けす
```


.statusに状態名を直接書き込むこともできます。
```
echo つける > /mnt/照明/.status
```


遊び終わったら、umountします。
```
umount /mnt
```


##21世紀にもなってコマンドラインは嫌ですか？
もちろん、Explorerでも表示できます。

![Explorer1](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/w2.jpg)

![Explorer2](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/w3.jpg)


ファイルをメモ帳とかのエディタで開いて、ctrl+s で保存すると、家電が動きます。
![メモ帳](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/w4.jpg)



#必要なもの
実行するには、「音声認識によるホームコントローラーであるフューチャーホームコントローラー」と、Linuxなら「fusefs」、Windowsなら「dokan」が必要です。


音声認識によるホームコントローラーであるフューチャーホームコントローラー
http://rti-giken.jp/


fusefs(linux)
http://fuse.sourceforge.net/


dokan(windows)
http://dokan-dev.net/en/



#遊び方
##1.
家電操作するためにフューチャーホームコントローラーを買ってきます。
![FHC](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/fhc.jpg)

http://rti-giken.jp/


##2.
ソースコードをダウンロードとかgit cloneとかなんでもいいから持ってきます。

このプログラムは、1ソースで複数プラットフォーム対応です。


##3.
環境を作ります。

*Linux(fusefs)*

fusefsとopensslとg++を入れます。
(ソースコードにあるopensslはwindows用に置いています。linuxではシステムのopensslを利用します。)


*Windows(dokan)*

dokanを入れます。
VS2010以降を入れます。


##4.
コンパイルします。

*Linux(fusefs)*

makeでコンパイルします。


*Windows(dokan)*

VS2010以降で homefs.sln を開いて、F5を押してビルドします。


##5.
フューチャーホームコントローラーのwebapi画面よりapikeyを取得します。
![FHC](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/f1.jpg)


##6.
mountします

###Linux(fuse)
```
./homefs 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX /mnt -o debug
```


###Windows(dokan)
```
homefs.exe 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX m -o debug
```



##7.
家電を操作したりして遊びます。



##8.
飽きたらumountしましょう。

*Linux(fuse)*
```
umount /mnt
```


*Windows(dokan)*
```
dokanctl.exe /u m
```


##9.
飯食いながら、アニメでも見て寝ます。



#このソフトウェアって意味あるの？
ないです。

家電操作に利用しているフューチャーホームコントローラーは、音声認識がありますので、ファイルで操作する暇があったら、「コンピュータ、電気消して」としゃべったほうが楽です。
スマホやタブレットでも操作できますし。
もし、何でもファイル経由で操作するのが便利であれば、[Plan9](http://ja.wikipedia.org/wiki/Plan_9)は覇権OSになっているでしょう。

現にそうなっていないということは、ファイルで表現するものが向いているもの(/procとか)と、向いていないものがあるということです。
何事もやり過ぎは良くないのです。

貧乳ツインテールは萌えかもしれませんが、巨乳ショートでロリババアで清楚ビッチでドジっ子メイドロボは、属性が多すぎて萌えるかどうかわからないのと同じです。



#ライセンス
以下のソフトウェアを利用しています。
openssl
dokan
fusefs


homefsは「NYSL」、「わっふるわっふると唱える」、「MIT」とかと「その他すべてのライセンス」とのマルチライセンスです。
好きなライセンスを選んでご自由にお使ください。


#作った人
rti7743  / [@super_rti](https://twitter.com/super_rti)

