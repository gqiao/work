class LineItem < ActiveRecord::Base
  belongs_to :order
  belongs_to :product
  belongs_to :cart

  attr_accessible :cart_id, :product_id, :quantity, :created_at, :id, :updated_at

  def total_price
    product.price * quantity
  end

end
