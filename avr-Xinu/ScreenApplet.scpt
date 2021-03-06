FasdUAS 1.101.10   ��   ��    k             l     ��  ��      ScreenApplet.scpt     � 	 	 $   S c r e e n A p p l e t . s c p t   
  
 l     ��  ��      Cocoa-AppleScript Applet     �   2   C o c o a - A p p l e S c r i p t   A p p l e t      l     ��������  ��  ��        l     ��  ��    + % Created by Michael Minor on 4/30/13.     �   J   C r e a t e d   b y   M i c h a e l   M i n o r   o n   4 / 3 0 / 1 3 .      l     ��  ��    + % Copyright 2013. All rights reserved.     �   J   C o p y r i g h t   2 0 1 3 .   A l l   r i g h t s   r e s e r v e d .      l     ��������  ��  ��        l     ��   ��    > 8 This is the main script for a Cocoa-AppleScript Applet.      � ! ! p   T h i s   i s   t h e   m a i n   s c r i p t   f o r   a   C o c o a - A p p l e S c r i p t   A p p l e t .   " # " l     �� $ %��   $ 7 1 Compile and Save as a script (ScreenApplet.scpt)    % � & & b   C o m p i l e   a n d   S a v e   a s   a   s c r i p t   ( S c r e e n A p p l e t . s c p t ) #  ' ( ' l     �� ) *��   ) $  run the script from Makefile:    * � + + <   r u n   t h e   s c r i p t   f r o m   M a k e f i l e : (  , - , l     �� . /��   . &  			osascript ./ScreenApplet.scpt    / � 0 0 @ 	 	 	 o s a s c r i p t   . / S c r e e n A p p l e t . s c p t -  1 2 1 l     �� 3 4��   3 B < create .screenrc file in your home directory (~/.screenrc):    4 � 5 5 x   c r e a t e   . s c r e e n r c   f i l e   i n   y o u r   h o m e   d i r e c t o r y   ( ~ / . s c r e e n r c ) : 2  6 7 6 l     �� 8 9��   8  			autodetach off    9 � : : " 	 	 	 a u t o d e t a c h   o f f 7  ; < ; l     �� = >��   = Z T Enable access for assistive devices in System Preferences > View > Universal Access    > � ? ? �   E n a b l e   a c c e s s   f o r   a s s i s t i v e   d e v i c e s   i n   S y s t e m   P r e f e r e n c e s   >   V i e w   >   U n i v e r s a l   A c c e s s <  @ A @ l     ��������  ��  ��   A  B C B l     �� D E��   D 6 0 Modify (Customize) Grass Terminal settings set:    E � F F `   M o d i f y   ( C u s t o m i z e )   G r a s s   T e r m i n a l   s e t t i n g s   s e t : C  G H G l     �� I J��   I j d Terminal > Preferences > Grass > Shell		Run command: sleep 4; screen /dev/tty.usbserial* 9600; exit    J � K K �   T e r m i n a l   >   P r e f e r e n c e s   >   G r a s s   >   S h e l l 	 	 R u n   c o m m a n d :   s l e e p   4 ;   s c r e e n   / d e v / t t y . u s b s e r i a l *   9 6 0 0 ;   e x i t H  L M L l     �� N O��   N " 												Run inside shell    O � P P 8 	 	 	 	 	 	 	 	 	 	 	 	 R u n   i n s i d e   s h e l l M  Q R Q l     �� S T��   S I C												When the shell exits: Close if the shell exited cleanly    T � U U � 	 	 	 	 	 	 	 	 	 	 	 	 W h e n   t h e   s h e l l   e x i t s :   C l o s e   i f   t h e   s h e l l   e x i t e d   c l e a n l y R  V W V l     ��������  ��  ��   W  X Y X l    
 Z���� Z O    
 [ \ [ I   	������
�� .miscactvnull��� ��� null��  ��   \ m      ] ]�                                                                                      @ alis    l  Macintosh HD               �+]�H+   ZeTerminal.app                                                    Z���i�        ����  	                	Utilities     �+�      �j!u     Ze Ze~  2Macintosh HD:Applications: Utilities: Terminal.app    T e r m i n a l . a p p    M a c i n t o s h   H D  #Applications/Utilities/Terminal.app   / ��  ��  ��   Y  ^ _ ^ l     ��������  ��  ��   _  ` a ` l   * b���� b O    * c d c I   )�� e��
�� .prcsclicuiel    ��� uiel e n    % f g f 4   " %�� h
�� 
menI h m   # $ i i � j j 
 G r a s s g n    " k l k 4    "�� m
�� 
menE m m     ! n n � o o  N e w   W i n d o w l n     p q p 4    �� r
�� 
menI r m     s s � t t  N e w   W i n d o w q n     u v u 4    �� w
�� 
menE w m     x x � y y 
 S h e l l v n     z { z 4    �� |
�� 
mbri | m     } } � ~ ~ 
 S h e l l { n      �  4    �� �
�� 
mbar � m    ����  � 4    �� �
�� 
pcap � m     � � � � �  T e r m i n a l��   d m     � ��                                                                                  sevs  alis    �  Macintosh HD               �+]�H+   ZeYSystem Events.app                                               []��j        ����  	                CoreServices    �+�      �3�     ZeY ZeL ZeK  =Macintosh HD:System: Library: CoreServices: System Events.app   $  S y s t e m   E v e n t s . a p p    M a c i n t o s h   H D  -System/Library/CoreServices/System Events.app   / ��  ��  ��   a  ��� � l     ��������  ��  ��  ��       �� � ���   � ��
�� .aevtoappnull  �   � **** � �� ����� � ���
�� .aevtoappnull  �   � **** � k     * � �  X � �  `����  ��  ��   �   �  ]�� ��� ����� }�� x�� s n i��
�� .miscactvnull��� ��� null
�� 
pcap
�� 
mbar
�� 
mbri
�� 
menE
�� 
menI
�� .prcsclicuiel    ��� uiel�� +� *j UO� *��/�k/��/��/��/��/��/j Uascr  ��ޭ