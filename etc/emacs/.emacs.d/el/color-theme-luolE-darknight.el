; Color theme support is required.
(require 'color-theme)

; Code start.
(defun color-theme-luolE-darknight ()
  (interactive)
  (color-theme-install
   '(color-theme-luolE-darknight
     ((background-color . "#121212")
      (foreground-color . "#F8F8F2")
      (cursor-color . "#DAD085"))
     (default ((t (nil))))
     (modeline ((t (:background "white" :foreground "black" :box (:line-width 1 :style released-button)))))
     (font-lock-builtin-face ((t (:foreground "#A6E22A"))))
     (font-lock-comment-face ((t (:foreground "#75715D"))))
     (font-lock-constant-face ((t (:foreground "#A6E22A"))))
     (font-lock-doc-string-face ((t (:foreground "#65B042"))))
     (font-lock-string-face ((t (:foreground "#DFD874"))))
     (font-lock-function-name-face ((t (:foreground "#00ff00" :bold t))))
     (font-lock-keyword-face ((t (:foreground "#66D9EF"))))
     (font-lock-type-face ((t (:underline t :foreground "#89BDFF"))))
     (font-lock-variable-name-face ((t (:foreground "#A6E22A"))))
     (font-lock-warning-face ((t (:bold t :foreground "#FD5FF1"))))
     (highlight-80+ ((t (:background "#252525"))))
     (hl-line ((t (:bold t :background "#252525"))))
	 (linum ((t (:foreground "#666666" ))))
     (region ((t (:background "#6DC5F1"))))
     (ido-subdir ((t (:foreground "#F1266F"))))
    )
  )
)
(provide 'color-theme-luolE-darknight)
;---------------
; Code end.