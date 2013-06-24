require 'digest/sha2'

class User < ActiveRecord::Base
  # attr_accessible :hashed_password, :name, :salt
  validates      :name,                 :presence => true, :uniqueness => true
  attr_reader    :password
  validates      :password,             :confirmation => true
  attr_accessor  :password_confirmation
  validate       :password_must_be_present
  after_destroy  :ensure_an_admin_remains

  def ensure_an_admin_remains
    if User.count.zero?
      raise "Can't delete last user"
    end
  end


  #
  # class.methods (classname.method) :
  #
  def User.authenticate(name, password)
    if user = find_by_name(name)
      if user.hashed_password == encrypt_password(password, user.salt)
        user
      end
    end
  end

  def User.encrypt_password(password, salt)
    Digest::SHA2.hexdigest(password + "wibble" + salt)
  end

  #
  # object.methods(self.method) :
  #
  def password=(password)
    @password = password
    if password.present?
      generate_salt
      self.hashed_password = self.class.encrypt_password(password, salt)
    end
  end


  private

  def password_must_be_present
    errors.add(:password, "Missing password") unless hashed_password.present?
  end

  def generate_salt
    self.salt = self.object_id.to_s + rand.to_s
  end

end
