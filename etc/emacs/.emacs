(setq org-export-with-sub-superscripts '{})
(setq initial-frame-alist '((top . 0) (left . 50) (width . 150) (height . 49)))

;; Guile
(load-file "~/.emacs.d/el/geiser/elisp/geiser.el")
;; byte-compiled
;; (load "~/.emacs.d/el/geiser/build/elisp/geiser-load")

;; SLIME
(load (expand-file-name "~/.emacs.d/el/quicklisp/slime-helper.el"))
;; Replace "sbcl" with the path to your implementation
(setq inferior-lisp-program "sbcl")

;; [~/.emacs.d/el]
(add-to-list 'load-path "~/.emacs.d/el")
(add-to-list 'load-path "~/.emacs.d/el/android-mode")
(add-to-list 'load-path "~/.emacs.d/el/color-theme")
(add-to-list 'load-path "~/.emacs.d/el/git-emacs")

(autoload 'company-mode "company" nil t)

(global-hl-line-mode t)

(ansi-color-for-comint-mode-on)

(when (fboundp 'global-font-lock-mode)
  (global-font-lock-mode t))
(setq diff-switches "-u")
(setq inhibit-startup-message t)


(setq frame-title-format (list "[ " (getenv "TARGET_NAME") "." (getenv "BRANCH_NAME") " ]  %f" ))
;;(setq frame-title-format '(" " (buffer-file-name "%f\ " (dired-directory dired-directory "%b\ "))))

(setq auto-mode-alist
      (cons
       '("\\.m$" . octave-mode)
       auto-mode-alist))


(custom-set-faces
  ;; custom-set-faces was added by Custom.
  ;; If you edit it by hand, you could mess it up, so be careful.
  ;; Your init file should contain only one such instance.
  ;; If there is more than one, they won't work right.
 )

(tool-bar-mode -1)
(setq column-number-mode t)
(scroll-bar-mode -1)
;;(menu-bar-mode nil)
;;(set-default-font "10x20")

;; [dos2unix]
;; C-x <RET> f then enter unix 

;; 2 — skip anything less than error
;; 1 — skip anything less than warning, or 
;; 0 — don’t skip any messages.
(setq compilation-skip-threshold 2)


;; M-(up, down, left, right)
(windmove-default-keybindings 'meta)
(show-paren-mode t)
(display-time-mode 1)    ;;启用时间显示设置，在minibuffer上面的那个杠上
(setq display-time-24hr-format t);;时间使用24小时制
(setq display-time-day-and-date t);;时间显示包括日期和具体时间
(setq display-time-interval 10);;时间的变化频率


(setq default-fill-column 80)
;;(setq default-major-mode 'text-mode)
(blink-cursor-mode -1)
(setq mouse-yank-at-point t)
;;(setq make-backup-files nil)
(setq backup-directory-alist '(("." . "/tmp")))
;;(setq-default cursor-type 'bar)
(fset 'yes-or-no-p 'y-or-n-p)



(defvar transparency-enabled nil)
;;(set-frame-parameter (selected-frame) 'alpha '(85 50))
(defun transparency-enable ()
  (interactive)
  (set-frame-parameter (selected-frame) 'alpha '(85 50))
  (setq transparency-enabled t))
(defun transparency-disable ()
  (interactive)
  (set-frame-parameter (selected-frame) 'alpha '(100 100))
  (setq transparency-enabled nil))
(defun transparency ()
  "Switch transparency"
  (interactive)
  (if transparency-enabled
      (transparency-disable)
    (transparency-enable)))


(require 'psvn)
(require 'git)
(require 'vc-git)
(require 'git-emacs)
(require 'google-define)
(require 'android-mode)


(defun k ()
  "Switch to kermit -c buffer"
  (interactive)
  (android-start-exclusive-command "*k*" (concat "sdcv") "")
  (switch-to-buffer "*k*"))

(require 'xcscope) 
(require 'multi-term)
(setq multi-term-program "/bin/zsh")
(custom-set-variables
 '(term-default-bg-color "#000000") 
 '(term-default-fg-color "white")) 

(require 'shell-pop)
(shell-pop-set-internal-mode "shell")
(shell-pop-set-internal-mode-shell "/bin/bash")
(shell-pop-set-window-height 60) ; the number for the percentage of the selected window.
(require 'kermit-c)
(kermit-c-set-internal-mode "terminal")
(kermit-c-set-internal-mode-shell "k")
(kermit-c-set-window-height 60) 
(require 'dict-pop)
(dict-pop-set-internal-mode "ansi-term")
(dict-pop-set-internal-mode-shell "sdcv")
(dict-pop-set-window-height 60) 

(require 'color-theme)
;;(require 'color-theme-luolE-darknight)
;;(color-theme-luolE-darknight)
(require 'color-theme-dark-george-laptop)
(color-theme-dark-george-laptop)
;;(set-frame-font "Monaco")

;;(color-theme-initialize)
;;(color-theme-dark-laptop)
;;(color-theme-robin-hood)

;; [TAGS]
(defun lev/find-tag (&optional show-only)
  "Show tag in other window with no prompt in minibuf."
  (interactive)
  (let ((default (funcall (or find-tag-default-function
                              (get major-mode 'find-tag-default-function)
                              'find-tag-default))))
    (if show-only
        (progn (find-tag-other-window default)
               (shrink-window (- (window-height) 12)) ;; 限制为 12 行
               (recenter 1)
               (other-window 1))
      (find-tag default))))

(defadvice lev/find-tag (around refresh-etags activate)
  "Rerun etag and reload tags if tag not found and redo find-tag.              
   If buffer is modified, ask about save before running etags."
  (let ((extension (file-name-extension (buffer-file-name))))
    (condition-case err ad-do-it
      (error (and (buffer-modified-p)
                  (not (ding)))
             (er-refresh-etags extension)
             ad-do-it))))
(defun er-refresh-etags (&optional extension)
  "Run etag on all peer files in current dir and reload them silently."
  (interactive)
  (shell-command (format "etag" (or extension "el")))
  (let ((tags-revert-without-query t))  ; don't query, revert silently 
    (visit-tags-table default-directory nil)))




;; [stardict]
(defun kid-sdcv-to-buffer ()
  (interactive)
  (let ((word (if mark-active
                  (buffer-substring-no-properties (region-beginning) (region-end))
                  (current-word nil t))))
    (setq word (read-string (format "Search the dictionary for (default %s): " word)
                            nil nil word))
    (set-buffer (get-buffer-create "*sdcv*"))
    (buffer-disable-undo)
    (erase-buffer)
    (let ((process (start-process-shell-command "sdcv" "*sdcv*" "sdcv" "-n" word)))
      (set-process-sentinel
       process
       (lambda (process signal)
         (when (memq (process-status process) '(exit signal))
           (unless (string= (buffer-name) "*sdcv*")
             (setq kid-sdcv-window-configuration (current-window-configuration))
             (switch-to-buffer-other-window "*sdcv*")
             (local-set-key (kbd "d") 'kid-sdcv-to-buffer)
             (local-set-key (kbd "q") (lambda ()
                                        (interactive)
                                        (bury-buffer)
                                        (unless (null (cdr (window-list))) ; only one window
                                          (delete-window)))))
           (goto-char (point-min))))))))


;;[toggle-window-split]
(defun toggle-window-split ()
  "Vertical split shows more of each line, horizontal split shows
more lines. This code toggles between them. It only works for
frames with exactly two windows."
  (interactive)
  (if (= (count-windows) 2)
      (let* ((this-win-buffer (window-buffer))
             (next-win-buffer (window-buffer (next-window)))
             (this-win-edges (window-edges (selected-window)))
             (next-win-edges (window-edges (next-window)))
             (this-win-2nd (not (and (<= (car this-win-edges)
                                         (car next-win-edges))
                                     (<= (cadr this-win-edges)
                                         (cadr next-win-edges)))))
             (splitter
              (if (= (car this-win-edges)
                     (car (window-edges (next-window))))
                  'split-window-horizontally
                'split-window-vertically)))
        (delete-other-windows)
        (let ((first-win (selected-window)))
          (funcall splitter)
          (if this-win-2nd (other-window 1))
          (set-window-buffer (selected-window) this-win-buffer)
          (set-window-buffer (next-window) next-win-buffer)
          (select-window first-win)
          (if this-win-2nd (other-window 1))))))


;; [ Left C-c ]
(global-set-key (kbd "C-c a")     'org-agenda)
(global-set-key (kbd "C-c b")     'speedbar)
(global-set-key (kbd "C-c c")     'compile)
(global-set-key (kbd "C-c d")     'kid-sdcv-to-buffer)
(global-set-key (kbd "C-c e")     'eval-current-buffer)
(global-set-key (kbd "C-c f")     'follow-mode)
;;(global-set-key (kbd "C-c g")     'gdb-many-windows)
(global-set-key (kbd "C-c g s")   'git-status)
(global-set-key (kbd "C-c g g")   'vc-git-grep)
(global-set-key (kbd "C-c l")     'slime) ;lisp
(global-set-key (kbd "C-c t")     'transparency)
(global-set-key (kbd "C-c s s")   'cscope-find-this-symbol)
(global-set-key (kbd "C-c s d")   'cscope-find-global-definition)
(global-set-key (kbd "C-c s i")   'cscope-find-files-including-file)
(global-set-key (kbd "C-c s u")   'cscope-pop-mark)
(global-set-key (kbd "C-c s f")   'cscope-find-this-file)
(global-set-key (kbd "C-c s E")   'cscope-edit-list-of-files-to-index)
(global-set-key (kbd "C-c s a")   'cscope-set-initial-directory)
(global-set-key (kbd "C-c |")     'toggle-window-split)

;; [ Right C-; ]
;;(global-set-key (kbd "C-; d")     'dict-pop)
;;(global-set-key (kbd "C-; k")     'kermit-c)
(global-set-key (kbd "C-; l")     'linum-mode)
(global-set-key (kbd "C-; ;")     'shell-pop)
(global-set-key (kbd "C-; C-;")   'multi-term)
(global-set-key (kbd "C-.")       '(lambda () (interactive) (lev/find-tag t)))
;;(global-set-key [(shift return)]  'complete-tag) ;; 查找 tag 自动补全


;; [Coding Style: C/C++]
(defconst pkj-cc-style
  '((c-tab-always-indent           . t)
    (c-comment-only-line-offset    . 0)
    (c-cleanup-list                . nil)
    (c-hanging-braces-alist        . ((brace-list-open)
				      ))
    (c-hanging-colons-alist        . (nil
				      ))
    (c-offsets-alist               . ((substatement-open . 0)
				      ))
    )
  "PKJ C Programming Style")


;; tab
;;(setq tab-width 8 c-basic-offset 8)
(setq-default indent-tabs-mode  nil)
(setq default-tab-width         4)
(setq-default tab-width         4)
(setq-default sh-basic-offset   4)
(setq-default sh-indentation    4)
(setq-default perl-indent-level 4)
(setq-default js-indent-level   4)
(setq css-indent-offset         4)


(add-hook 'c-mode-common-hook
          (lambda()
	    (c-add-style "PKJ" pkj-cc-style t)	; offset customizations not in my-c-style
	    (c-set-offset 'member-init-intro '++) 
	    (setq tab-width 4)			; tabs are *always* 8 spaces
	    ;;(setq indent-tabs-mode t)		; use tabs instead of spaces
	    (define-key c-mode-map "\C-m" 'newline-and-indent)	; indent on newline
	    ))

(setq cscope-initial-directory (getenv "PWD"))

(if (file-exists-p "TAGS") 
    (visit-tags-table "TAGS"))

(setq tags-revert-without-query t)

