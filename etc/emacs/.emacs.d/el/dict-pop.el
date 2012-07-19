;;; dict-pop.el --- Helps you pop up and pop out shell buffer easily.
;; You can choose your favorite internal mode such as `shell', `terminal',
;; `ansi-term', and `eshell'. Also you can use any shell such as
;; `/bin/bash', `/bin/tcsh', `/bin/zsh' as you like.
;;
;; A configuration sample for your .emacs is as follows.
;;
;; (require 'dict-pop)
;; (dict-pop-set-internal-mode "ansi-term")
;; (dict-pop-set-internal-mode-shell "/bin/zsh")
;; (global-set-key [f8] 'dict-pop)
;;
;; Besides, you can set the window height, the number for the percentage
;; for selected window.
;;
;; (dict-pop-set-window-height 60)
;;
;;

;;; Code:
(defvar dict-pop-last-buffer nil)
(defvar dict-pop-last-window nil)
(defvar dict-pop-window-height 30) ; percentage for shell-buffer window height

(defvar dict-pop-internal-mode "shell")
(defvar dict-pop-internal-mode-buffer "*shell*")
(defvar dict-pop-internal-mode-func '(lambda () (shell)))
(defvar dict-pop-internal-mode-shell "/bin/bash")

(defvar dict-pop-internal-mode-list
  (list
    ; mode, buffer, function
    '("shell"     "*shell*"     '(lambda () (shell)))
    '("terminal"  "*terminal*"  '(lambda () (term dict-pop-internal-mode-shell)))
    '("ansi-term" "*ansi-term*" '(lambda () (ansi-term dict-pop-internal-mode-shell)))
    '("eshell"    "*eshell*"    '(lambda () (eshell)))))

(defun dict-pop-set-window-height (number)
  (interactive "nInput the number for the percentage of \
selected window height (10-90): ")
  (setq dict-pop-window-height number))

(defun dict-pop-set-internal-mode (mode)
  (interactive "sInput your favorite mode (shell|terminal|ansi-term|eshell): ")
  (if (catch 'found
        (dolist (l dict-pop-internal-mode-list)
          (if (string-match mode (car l))
              (progn
                (setq dict-pop-internal-mode-buffer (nth 1 l))
                (setq dict-pop-internal-mode-func (nth 2 l))
                (throw 'found t)))))
      t
    nil))

(defun dict-pop-set-internal-mode-shell (shell)
  (interactive (list (read-from-minibuffer "Input your favorite shell:"
                                           dict-pop-internal-mode-shell)))
  (setq dict-pop-internal-mode-shell shell))

(defun dict-pop ()
  (interactive)
  (if (equal (buffer-name) dict-pop-internal-mode-buffer)
      (dict-pop-out)
    (dict-pop-up)))

(defun dict-pop-up ()
  (let ((w (get-buffer-window dict-pop-internal-mode-buffer)))
    (if w
        (select-window w)
      (progn
        ; save dict-pop-last-buffer and dict-pop-last-window to return
          (setq dict-pop-last-buffer (buffer-name))
          (setq dict-pop-last-window (selected-window))
          (split-window (selected-window)
                        (round (* (window-height)
                                  (/ (- 100 dict-pop-window-height) 100.0))))
          (other-window 1)
          (if (not (get-buffer dict-pop-internal-mode-buffer))
              (funcall (eval dict-pop-internal-mode-func))
            (switch-to-buffer dict-pop-internal-mode-buffer))))))

(defun dict-pop-out ()
  (delete-window)
  (select-window dict-pop-last-window)
  (switch-to-buffer dict-pop-last-buffer))

(provide 'dict-pop)

;;; dict-pop.el ends here.
