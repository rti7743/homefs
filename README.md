-��mount- homefs
=============================

#�T�v
�Ƃ�mount���܂��B

![���S](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/icon.jpg)

 
 
 
 
 

#�������Ă���񂾂��̐l�E�E�E
�Ƃ��t�@�C���V�X�e���Ƃ���mount���܂��B

*Linux(fusefs)*
�Ƃ�/mnt��mount���܂��B
```
./homefs 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX /mnt -o debug
```


*Windows(dokan)*
�Ƃ�m:�h���C�u��mount���܂��B
```
homefs.exe 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX m -o debug
```


#�Ƃ��t�@�C���V�X�e��������̂ł��B
mount�������Ƃ́Als(windows����dir) �� echo �A cat(windows����type)�ŉƂɃA�N�Z�X�ł��܂��B


```
mount���āA
./homefs 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX /mnt/
```


ls �ŉƓd���\������܂��A
```
ls -la /mnt/
drwxr-xr-x  5 root root    0  1��  1  1970 .
drwxr-xr-x 23 root root 4096 12��  8 15:39 ..
drwxr-xr-x  7 root root    0  1��  1  1970 �G�A�R��
drwxr-xr-x  4 root root    0  1��  1  1970 �Ɩ�
drwxr-xr-x  4 root root    0  1��  1  1970 ��@
```


�Ɠd�f�B���N�g���̒��ɂ́A���얼������܂��B
```
ls -la /mnt/�Ɩ�
drwxr-xr-x 4 root root 0  1��  1  1970 .
drwxr-xr-x 5 root root 0  1��  1  1970 ..
-rwxr--r-- 1 root root 9  1��  1  1970 .status
-rwxr--r-- 1 root root 0  1��  1  1970 ����
-rwxr--r-- 1 root root 1  1��  1  1970 ����
```


��Ԃ�1��on�Ƃ��̕�������������ނƁA���̏�ԂɂȂ�܂��B
�Ⴆ�΁A�Ɩ��������ɂ́A���̂悤�ɂ��܂��B
```
echo 1 >/mnt/�Ɩ�/����
```


���̏�Ԃ́A.status������΂킩��܂��B
```
cat /mnt/�Ɩ�/.status

����
```


.status�ɏ�Ԗ��𒼐ڏ������ނ��Ƃ��ł��܂��B
```
echo ���� > /mnt/�Ɩ�/.status
```


�V�яI�������Aumount���܂��B
```
umount /mnt
```


##21���I�ɂ��Ȃ��ăR�}���h���C���͌��ł����H
�������AExplorer�ł��\���ł��܂��B

![Explorer1](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/w2.jpg)

![Explorer2](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/w3.jpg)


�t�@�C�����������Ƃ��̃G�f�B�^�ŊJ���āActrl+s �ŕۑ�����ƁA�Ɠd�������܂��B
![������](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/w4.jpg)



#�K�v�Ȃ���
���s����ɂ́A�u�����F���ɂ��z�[���R���g���[���[�ł���t���[�`���[�z�[���R���g���[���[�v�ƁALinux�Ȃ�ufusefs�v�AWindows�Ȃ�udokan�v���K�v�ł��B


�����F���ɂ��z�[���R���g���[���[�ł���t���[�`���[�z�[���R���g���[���[

http://rti-giken.jp/


fusefs(linux)

http://fuse.sourceforge.net/


dokan(windows)

http://dokan-dev.net/en/



#�V�ѕ�
##1.
�Ɠd���삷�邽�߂Ƀt���[�`���[�z�[���R���g���[���[�𔃂��Ă��܂��B
![FHC](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/fhc.jpg)

http://rti-giken.jp/


##2.
�\�[�X�R�[�h���_�E�����[�h�Ƃ�git clone�Ƃ��Ȃ�ł��������玝���Ă��܂��B

���̃v���O�����́A1�\�[�X�ŕ����v���b�g�t�H�[���Ή��ł��B


##3.
�������܂��B

*Linux(fusefs)*
fusefs��openssl��g++�����܂��B
(�\�[�X�R�[�h�ɂ���openssl��windows�p�ɒu���Ă��܂��Blinux�ł̓V�X�e����openssl�𗘗p���܂��B)


*Windows(dokan)*
dokan�����܂��B
VS2010�ȍ~�����܂��B


##4.
�R���p�C�����܂��B

*Linux(fusefs)*
make�ŃR���p�C�����܂��B


*Windows(dokan)*
VS2010�ȍ~�� homefs.sln ���J���āAF5�������ăr���h���܂��B


##5.
�t���[�`���[�z�[���R���g���[���[��webapi��ʂ��apikey���擾���܂��B
![FHC](https://raw.githubusercontent.com/rti7743/homefs/master/docimage/f1.jpg)


##6.
mount���܂�

###Linux(fuse)
```
./homefs 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX /mnt -o debug
```


###Windows(dokan)
```
homefs.exe 192.168.10.21@webapi_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX m -o debug
```



##7.
�Ɠd�𑀍삵���肵�ėV�т܂��B



##8.
�O������umount���܂��傤�B

*Linux(fuse)*
```
umount /mnt
```


*Windows(dokan)*
```
dokanctl.exe /u m
```


##9.
�ѐH���Ȃ���A�A�j���ł����ĐQ�܂��B



#���̃\�t�g�E�F�A���ĈӖ�����́H
�Ȃ��ł��B

�Ɠd����ɗ��p���Ă���t���[�`���[�z�[���R���g���[���[�́A�����F��������܂��̂ŁA�t�@�C���ő��삷��ɂ���������A�u�R���s���[�^�A�d�C�����āv�Ƃ���ׂ����ق����y�ł��B�X�}�z��^�u���b�g�ł�����ł��܂����B
�����A���ł��t�@�C���o�R�ő��삷��̂��֗��ł���΁APlan9�͔e��OS�ɂȂ��Ă���ł��傤�B
���ɂ����Ȃ��Ă��Ȃ��Ƃ������Ƃ́A�t�@�C���ŕ\��������̂������Ă������(/proc�Ƃ�)�ƁA�����Ă��Ȃ����̂�����Ƃ������Ƃł��B
���������߂��͗ǂ��Ȃ��̂ł��B
�n���c�C���e�[���͖G����������܂��񂪁A�����V���[�g�Ń����o�o�A�Ő��^�r�b�`�Ńh�W���q���C�h���{�͖G���邩�ǂ����킩��Ȃ��Ƃ������Ƃł��B



#���C�Z���X
�ȉ��̃\�t�g�E�F�A�𗘗p���Ă��܂��B
openssl
dokan
fusefs

homefs�́uNYSL�v�A�u����ӂ����ӂ�Ə�����v�A�uMIT�v�Ƃ��Ɓu���̑����ׂẴ��C�Z���X�v�Ƃ̃}���`���C�Z���X�ł��B
�D���ȃ��C�Z���X��I��ł����R�ɂ��g���������B


#������l
rti7743  / @super_rti

