if defined?(ChefSpec)
  ChefSpec.define_matcher :kernel_module

  def install_kernel_module(mod)
    ChefSpec::Matchers::ResourceMatcher.new(:kernel_module, :install, mod)
  end

  def uninstall_kernel_module(mod)
    ChefSpec::Matchers::ResourceMatcher.new(:kernel_module, :uninstall, mod)
  end

  def load_kernel_module(mod)
    ChefSpec::Matchers::ResourceMatcher.new(:kernel_module, :load, mod)
  end

  def unload_kernel_module(mod)
    ChefSpec::Matchers::ResourceMatcher.new(:kernel_module, :unload, mod)
  end

  def blacklist_kernel_module(mod)
    ChefSpec::Matchers::ResourceMatcher.new(:kernel_module, :blacklist, mod)
  end

end
