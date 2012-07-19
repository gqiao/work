;;; kermit-c.el --- Helps you pop up and pop out shell buffer easily.
;; You can choose your favorite internal mode such as `shell', `terminal',
;; `ansi-term', and `eshell'. Also you can use any shell such as
;; `/bin/bash', `/bin/tcsh', `/bin/zsh' as you like.
;;
;; A configuration sample for your .emacs is as follows.
;;
;; (require 'kermit-c)
;; (kermit-c-set-internal-mode "ansi-term")
;; (kermit-c-set-internal-mode-shell "/bin/zsh")
;; (global-set-key [f8] 'kermit-c)
;;
;; Besides, you can set the window height, the number for the percentage
;; for selected window.
;;
;; (kermit-c-set-window-height 60)
;;
;;

;;; Code:
(defvar kermit-c-last-buffer nil)
(defvar kermit-c-last-window nil)
(defvar kermit-c-window-height 30) ; percentage for shell-buffer window height

(defvar kermit-c-internal-mode "shell")
(defvar kermit-c-internal-mode-buffer "*shell*")
(defvar kermit-c-internal-mode-func '(lambda () (shell)))
(defvar kermit-c-internal-mode-shell "/bin/bash")

(defvar kermit-c-internal-mode-list
  (list
    ; mode, buffer, function
    '("shell"     "*shell*"     '(lambda () (shell)))
    '("terminal"  "*terminal*"  '(lambda () (term kermit-c-internal-mode-shell)))
    '("ansi-term" "*ansi-term*" '(lambda () (ansi-term kermit-c-internal-mode-shell)))
    '("eshell"    "*eshell*"    '(lambda () (eshell)))))

(defun kermit-c-set-window-height (number)
  (interactive "nInput the number for the percentage of \
selected window height (10-90): ")
  (setq kermit-c-window-height number))

(defun kermit-c-set-internal-mode (mode)
  (interactive "sInput your favorite mode (shell|terminal|ansi-term|eshell): ")
  (if (catch 'found
        (dolist (l kermit-c-internal-mode-list)
          (if (string-match mode (car l))
              (progn
                (setq kermit-c-internal-mode-buffer (nth 1 l))
                (setq kermit-c-internal-mode-func (nth 2 l))
                (throw 'found t)))))
      t
    nil))

(defun kermit-c-set-internal-mode-shell (shell)
  (interactive (list (read-from-minibuffer "Input your favorite shell:"
                                           kermit-c-internal-mode-shell)))
  (setq kermit-c-internal-mode-shell shell))

(defun kermit-c ()
  (interactive)
  (if (equal (buffer-name) kermit-c-internal-mode-buffer)
      (kermit-c-out)
    (kermit-c-up)))

(defun kermit-c-up ()
  (let ((w (get-buffer-window kermit-c-internal-mode-buffer)))
    (if w
        (select-window w)
      (progn
        ; save kermit-c-last-buffer and kermit-c-last-window to return
          (setq kermit-c-last-buffer (buffer-name))
          (setq kermit-c-last-window (selected-window))
          (split-window (selected-window)
                        (round (* (window-height)
                                  (/ (- 100 kermit-c-window-height) 100.0))))
          (other-window 1)
          (if (not (get-buffer kermit-c-internal-mode-buffer))
              (funcall (eval kermit-c-internal-mode-func))
            (switch-to-buffer kermit-c-internal-mode-buffer))))))

(defun kermit-c-out ()
  (delete-window)
  (select-window kermit-c-last-window)
  (switch-to-buffer kermit-c-last-buffer))

(provide 'kermit-c)

;;; kermit-c.el ends here.
