#include "context.hpp"
#include "../exception.hpp"
#include "../log.hpp"
#include "graphics_pipeline.hpp"
#include <array>
#include <cstring>
#include <limits>
#include <set>

PFN_vkCreateDebugUtilsMessengerEXT
    core::vulkan::Context::pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT
    core::vulkan::Context::pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pMessenger) {
  return core::vulkan::Context::pfnVkCreateDebugUtilsMessengerEXT(
      instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT messenger,
    VkAllocationCallbacks const *pAllocator) {
  return core::vulkan::Context::pfnVkDestroyDebugUtilsMessengerEXT(
      instance, messenger, pAllocator);
}

namespace core {
namespace vulkan {
Context::Context(Window &window)
    : m_current_frame(0), m_framebuffer_resized(false) {
  const std::vector<const char *> validation_layers = {
      "VK_LAYER_KHRONOS_validation"};
  _create_instance(window, validation_layers);
  _setup_debug_messenger();
  _create_surface(window);
  {
    const std::vector<const char *> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    _pick_physical_device(device_extensions);
    _create_logical_device(device_extensions, validation_layers);
  }
  _create_command_pool();
  _create_swap_chain(window);
  _allocate_command_buffers();
  _create_sync_objects();

  // Handle framebuffer resize
  window.set_on_resize([&](auto, auto) { m_framebuffer_resized = true; });

  Log::info("Successfully Constructed Vulkan Context");
}

Context::~Context() {
  m_device.waitIdle();

  m_swap_chain.reset();

  for (size_t i = 0; i < _max_images_in_flight; i++) {
    m_device.destroySemaphore(m_render_finished_semaphores[i]);
    m_device.destroySemaphore(m_image_available_semaphores[i]);
    m_device.destroyFence(m_in_flight_fences[i]);
  }

  m_device.destroyCommandPool(m_graphic_command_pool);

  m_device.destroy();
  if constexpr (_enable_validation_layers) {
    m_instance.destroyDebugUtilsMessengerEXT(m_debug_messenger);
  }
  m_instance.destroySurfaceKHR(m_surface);
  m_instance.destroy();
}

std::optional<RenderCall> Context::render_begin() {
  if (m_device.waitForFences(m_in_flight_fences[m_current_frame], VK_TRUE,
                             std::numeric_limits<uint64_t>::max()) !=
      vk::Result::eSuccess) {
    return std::nullopt;
  }

  auto acquired_image = m_swap_chain->acquire_next_image(
      m_device, m_image_available_semaphores[m_current_frame]);
  if (!acquired_image) {
    _handle_framebuffer_resize();
    return std::nullopt;
  }

  auto [image_index, framebuffer] = acquired_image.value();

  if (m_images_in_flight[image_index]) {
    if (m_device.waitForFences(m_images_in_flight[image_index], VK_TRUE,
                               std::numeric_limits<uint64_t>::max()) !=
        vk::Result::eSuccess) {
      return std::nullopt;
    }
  }
  m_images_in_flight[image_index] = m_in_flight_fences[m_current_frame];

  m_graphic_command_buffers[image_index].begin(vk::CommandBufferBeginInfo());

  vk::RenderPassBeginInfo rbi;
  rbi.renderPass = m_swap_chain->get_render_pass();
  rbi.framebuffer = framebuffer;
  rbi.renderArea.extent = m_swap_chain->get_extent();

  std::array<vk::ClearValue, 2> clear_values;
  clear_values[0].color = std::array{
      0.0f,
      0.0f,
      0.0f,
      1.0f,
  };
  clear_values[1].depthStencil.depth = 1.0f;
  rbi.clearValueCount = static_cast<uint32_t>(clear_values.size());
  rbi.pClearValues = clear_values.data();

  m_graphic_command_buffers[image_index].beginRenderPass(
      rbi, vk::SubpassContents::eInline);

  vk::Viewport view;
  view.width = static_cast<float>(m_swap_chain->get_extent().width);
  view.height = static_cast<float>(m_swap_chain->get_extent().height);
  view.minDepth = 0.0f;
  view.maxDepth = 1.0f;
  view.x = 0.0f;
  view.y = 0.0f;

  vk::Rect2D scissor;
  scissor.extent = m_swap_chain->get_extent();

  m_graphic_command_buffers[image_index].setViewport(0, view);
  m_graphic_command_buffers[image_index].setScissor(0, scissor);

  return RenderCall(m_graphic_command_buffers[image_index], image_index);
}

void Context::render_end() {
  if (!m_swap_chain->get_current_image()) {
    return;
  }
  const auto image_index = m_swap_chain->get_current_image().value();
  m_graphic_command_buffers[image_index].endRenderPass();
  m_graphic_command_buffers[image_index].end();

  vk::SubmitInfo si;

  const auto wait_sems =
      std::array{m_image_available_semaphores[m_current_frame]};
  const auto wait_stages = std::array<vk::PipelineStageFlags, wait_sems.size()>{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};
  si.waitSemaphoreCount = static_cast<uint32_t>(wait_sems.size());
  si.pWaitSemaphores = wait_sems.data();
  si.pWaitDstStageMask = wait_stages.data();
  si.commandBufferCount = 1;
  si.pCommandBuffers = &m_graphic_command_buffers[image_index];

  const auto sig_sems =
      std::array{m_render_finished_semaphores[m_current_frame]};
  si.signalSemaphoreCount = static_cast<uint32_t>(sig_sems.size());
  si.pSignalSemaphores = sig_sems.data();

  m_device.resetFences(m_in_flight_fences[m_current_frame]);

  m_graphics_queue.submit(si, m_in_flight_fences[m_current_frame]);

  vk::PresentInfoKHR pi;
  pi.waitSemaphoreCount = static_cast<uint32_t>(sig_sems.size());
  pi.pWaitSemaphores = sig_sems.data();

  const auto swap_chains = std::array{m_swap_chain->get_handle()};
  pi.swapchainCount = static_cast<uint32_t>(swap_chains.size());
  pi.pSwapchains = swap_chains.data();
  pi.pImageIndices = &image_index;

  if (const auto r = m_present_queue.presentKHR(pi);
      r == vk::Result::eErrorOutOfDateKHR || r == vk::Result::eSuboptimalKHR ||
      m_framebuffer_resized) {
    _handle_framebuffer_resize();
    m_framebuffer_resized = false;
  }

  m_current_frame = (m_current_frame + 1) % _max_images_in_flight;
}

vk::DescriptorSetLayout Context::create_descriptor_set_layout(
    std::vector<vk::DescriptorSetLayoutBinding> bindings) const {
  vk::DescriptorSetLayoutCreateInfo li;
  li.bindingCount = static_cast<uint32_t>(bindings.size());
  li.pBindings = bindings.data();

  return m_device.createDescriptorSetLayout(li);
}

vk::DescriptorPool
Context::create_descriptor_pool(std::vector<vk::DescriptorPoolSize> pool_sizes,
                                const size_t set_count) const {
  vk::DescriptorPoolCreateInfo pi;
  pi.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pi.pPoolSizes = pool_sizes.data();
  pi.maxSets = static_cast<uint32_t>(set_count);

  return m_device.createDescriptorPool(pi);
}

std::vector<vk::DescriptorSet>
Context::create_descriptor_sets(const vk::DescriptorPool &pool,
                                const vk::DescriptorSetLayout &layout) const {
  std::vector<vk::DescriptorSetLayout> layouts(get_swap_chain_image_count(),
                                               layout);
  vk::DescriptorSetAllocateInfo ai;
  ai.descriptorPool = pool;
  ai.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  ai.pSetLayouts = layouts.data();

  return m_device.allocateDescriptorSets(ai);
}

void Context::write_descriptor_sets(
    std::vector<vk::WriteDescriptorSet> writes) const noexcept {
  m_device.updateDescriptorSets(std::move(writes), nullptr);
}

void Context::destroy_descriptors(
    vk::DescriptorPool pool, vk::DescriptorSetLayout layout) const noexcept {
  m_device.destroyDescriptorPool(pool);
  m_device.destroyDescriptorSetLayout(layout);
}

Context::QueueFamilyIndices::QueueFamilyIndices(
    const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface) {
  const auto queue_families(device.getQueueFamilyProperties());

  int i = 0;
  for (const auto &qf : queue_families) {
    if (qf.queueFlags & vk::QueueFlagBits::eGraphics) {
      graphics_family = i;
    }

    const auto present_support = device.getSurfaceSupportKHR(i, surface);
    if (present_support) {
      present_family = i;
    }

    if (is_complete()) {
      break;
    }

    i++;
  }
}

bool Context::QueueFamilyIndices::is_complete() const {
  return graphics_family.has_value() && present_family.has_value();
}

Context::SwapChainSupportDetails::SwapChainSupportDetails(
    const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
    : capabilities(device.getSurfaceCapabilitiesKHR(surface)),
      formats(device.getSurfaceFormatsKHR(surface)),
      present_modes(device.getSurfacePresentModesKHR(surface)) {}

bool Context::_has_validation_layer_support(
    const std::vector<const char *> &layer_names) noexcept {
  const auto layers(vk::enumerateInstanceLayerProperties());

  for (const auto *ln : layer_names) {
    bool found = false;
    for (const auto &l : layers) {
      if (strcmp(l.layerName.data(), ln) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }

  return true;
}

void Context::_populate_debug_messenger_create_info(
    vk::DebugUtilsMessengerCreateInfoEXT &di) noexcept {
  di.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                       vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                       vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
  di.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                   vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                   vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
  di.pfnUserCallback = _debug_callback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Context::_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  std::string msg("\033[32m[Validation Layers]\033[0m ");
  msg += pCallbackData->pMessage;

  switch (
      static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) {
  case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
    Log::error(msg);
    break;
  case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
    Log::warning(msg);
    break;
  default:
    Log::info(msg);
    break;
  }

  return VK_FALSE;
}

bool Context::_is_device_suitable(
    const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface,
    const std::vector<const char *> &extension_names) {
  const QueueFamilyIndices indices(device, surface);

  const auto extensions_supported =
      _device_has_extension_support(device, extension_names);
  bool swap_chain_adequate = false;
  if (extensions_supported) {
    const SwapChainSupportDetails swap_chain_support(device, surface);
    swap_chain_adequate = !swap_chain_support.formats.empty() &&
                          !swap_chain_support.present_modes.empty();
  }

  const auto features = device.getFeatures();

  return indices.is_complete() && extensions_supported && swap_chain_adequate &&
         features.samplerAnisotropy;
}

bool Context::_device_has_extension_support(
    const vk::PhysicalDevice &device,
    const std::vector<const char *> &extension_names) {
  const auto extensions(device.enumerateDeviceExtensionProperties());

  for (const auto *en : extension_names) {
    bool found = false;
    for (const auto &e : extensions) {
      if (strcmp(e.extensionName.data(), en) == 0) {
        found = true;
      }
    }
    if (!found)
      return false;
  }

  return true;
}

vk::Format Context::_find_supported_format(
    const vk::PhysicalDevice &device, const std::vector<vk::Format> &candidates,
    vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
  for (const auto &format : candidates) {
    const auto props{device.getFormatProperties(format)};
    if (tiling == vk::ImageTiling::eLinear &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == vk::ImageTiling::eOptimal &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw VulkanKraftException("failed to find supported format");
}

vk::Format Context::_find_depth_format(const vk::PhysicalDevice &device) {
  return _find_supported_format(
      device,
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
       vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

uint32_t Context::_find_memory_type(const vk::PhysicalDevice &device,
                                    uint32_t type_filter,
                                    vk::MemoryPropertyFlags props) {
  const auto mem_props(device.getMemoryProperties());
  for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
    if (type_filter & (1 << i) &&
        (mem_props.memoryTypes[i].propertyFlags & props) == props) {
      return i;
    }
  }

  throw VulkanKraftException("failed to find suitable memory type");
}

vk::CommandBuffer
Context::_begin_single_time_commands(const vk::Device &device,
                                     const vk::CommandPool &command_pool) {
  vk::CommandBufferAllocateInfo ai;
  ai.level = vk::CommandBufferLevel::ePrimary;
  ai.commandPool = command_pool;
  ai.commandBufferCount = 1;

  vk::CommandBuffer cb;
  try {
    auto cbs = device.allocateCommandBuffers(ai);
    cb = cbs[0];
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string(
            "failed to allocate command buffer for single time commands: ") +
        e.what());
  }

  vk::CommandBufferBeginInfo cbi;
  cbi.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

  cb.begin(cbi);

  return cb;
}

void Context::_end_single_time_commands(const vk::Device &device,
                                        const vk::CommandPool &command_pool,
                                        const vk::Queue &queue,
                                        vk::CommandBuffer command_buffer) {
  command_buffer.end();

  vk::SubmitInfo si;
  si.commandBufferCount = 1;
  si.pCommandBuffers = &command_buffer;

  queue.submit(si);
  queue.waitIdle();

  device.freeCommandBuffers(command_pool, command_buffer);
}

void Context::_transition_image_layout(const vk::Image &image,
                                       vk::Format format,
                                       vk::ImageLayout old_layout,
                                       vk::ImageLayout new_layout,
                                       uint32_t mip_levels) const {
  auto com_buf(_begin_single_time_commands(m_device, m_graphic_command_pool));

  vk::ImageMemoryBarrier bar;
  bar.oldLayout = old_layout;
  bar.newLayout = new_layout;
  bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  bar.image = image;
  if (new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
    bar.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

    if (_has_stencil_component(format)) {
      bar.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
    }
  } else {
    bar.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  }
  bar.subresourceRange.baseMipLevel = 0;
  bar.subresourceRange.levelCount = mip_levels;
  bar.subresourceRange.baseArrayLayer = 0;
  bar.subresourceRange.layerCount = 1;
  bar.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
  bar.dstAccessMask = vk::AccessFlagBits::eNoneKHR;

  vk::PipelineStageFlags src_stage, dst_stage;
  if (old_layout == vk::ImageLayout::eUndefined &&
      new_layout == vk::ImageLayout::eTransferDstOptimal) {
    bar.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
    bar.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    dst_stage = vk::PipelineStageFlagBits::eTransfer;
  } else if (old_layout == vk::ImageLayout::eTransferDstOptimal &&
             new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    bar.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    bar.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    src_stage = vk::PipelineStageFlagBits::eTransfer;
    dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
  } else if (old_layout == vk::ImageLayout::eUndefined &&
             new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
    bar.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
    bar.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead |
                        vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
  } else {
    throw VulkanKraftException("unsupported layout transistion");
  }

  com_buf.pipelineBarrier(src_stage, dst_stage,
                          static_cast<vk::DependencyFlagBits>(0), nullptr,
                          nullptr, bar);

  _end_single_time_commands(m_device, m_graphic_command_pool, m_graphics_queue,
                            std::move(com_buf));
}

void Context::_create_instance(
    const Window &window, const std::vector<const char *> &validation_layers) {

  if (_enable_validation_layers &&
      !_has_validation_layer_support(validation_layers)) {
    throw VulkanKraftException("validation layers are not supported");
  }

  vk::ApplicationInfo ai;
  ai.pApplicationName = _application_name;
  ai.applicationVersion = _application_version;
  ai.pEngineName = _engine_name;
  ai.engineVersion = _engine_version;
  ai.apiVersion = VK_API_VERSION_1_0;

  vk::InstanceCreateInfo ii;
  ii.pApplicationInfo = &ai;

  auto extensions = window.get_required_vulkan_extensions();
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  ii.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  ii.ppEnabledExtensionNames = extensions.data();

  vk::DebugUtilsMessengerCreateInfoEXT di;
  if constexpr (_enable_validation_layers) {
    ii.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    ii.ppEnabledLayerNames = validation_layers.data();

    _populate_debug_messenger_create_info(di);
    ii.pNext = &di;
  }

  try {
    m_instance = vk::createInstance(ii);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(std::string("failed to create instance: ") +
                               e.what());
  }
}

void Context::_setup_debug_messenger() {
  pfnVkCreateDebugUtilsMessengerEXT =
      reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          m_instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
  if (!pfnVkCreateDebugUtilsMessengerEXT) {
    throw VulkanKraftException("GetInstanceProcAddr: Unable to find "
                               "pfnVkCreateDebugUtilsMessengerEXT function.");
  }

  pfnVkDestroyDebugUtilsMessengerEXT =
      reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
          m_instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
  if (!pfnVkDestroyDebugUtilsMessengerEXT) {
    throw VulkanKraftException("GetInstanceProcAddr: Unable to find "
                               "pfnVkDestroyDebugUtilsMessengerEXT function.");
  }

  if constexpr (_enable_validation_layers) {
    vk::DebugUtilsMessengerCreateInfoEXT di;
    _populate_debug_messenger_create_info(di);

    try {
      m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(di);
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(
          std::string("failed to set up debug messenger: ") + e.what());
    }
  }
}

void Context::_create_surface(const Window &window) {
  m_surface = window.create_vulkan_surface(m_instance);
}

void Context::_pick_physical_device(
    const std::vector<const char *> &extensions) {
  const auto devices(m_instance.enumeratePhysicalDevices());
  if (devices.empty()) {
    throw VulkanKraftException("no GPUs with vulkan support found");
  }

  for (const auto &d : devices) {
    if (_is_device_suitable(d, m_surface, extensions)) {
      m_physical_device = d;
    }
  }

  if (!m_physical_device) {
    throw VulkanKraftException("failed to find a suitable GPU");
  }
}

void Context::_create_logical_device(
    const std::vector<const char *> &extensions,
    const std::vector<const char *> &validation_layers) {
  const QueueFamilyIndices indices(m_physical_device, m_surface);
  std::vector<vk::DeviceQueueCreateInfo> qis;
  std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(),
                                              indices.present_family.value()};

  const float queue_priority = 1.0f;
  for (const auto &qf : unique_queue_families) {
    vk::DeviceQueueCreateInfo qi;
    qi.queueFamilyIndex = qf;
    qi.queueCount = 1;
    qi.pQueuePriorities = &queue_priority;
    qis.emplace_back(std::move(qi));
  }

  vk::PhysicalDeviceFeatures features;
  features.samplerAnisotropy = VK_FALSE;
  features.sampleRateShading = VK_FALSE;

  vk::DeviceCreateInfo di;
  di.queueCreateInfoCount = static_cast<uint32_t>(qis.size());
  di.pQueueCreateInfos = qis.data();
  di.pEnabledFeatures = &features;
  di.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  di.ppEnabledExtensionNames = extensions.data();
  if constexpr (_enable_validation_layers) {
    di.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    di.ppEnabledLayerNames = validation_layers.data();
  }

  try {
    m_device = m_physical_device.createDevice(di);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create logical device: ") + e.what());
  }

  m_graphics_queue = m_device.getQueue(indices.graphics_family.value(), 0);
  m_present_queue = m_device.getQueue(indices.present_family.value(), 0);
}

void Context::_create_command_pool() {
  const QueueFamilyIndices indices(m_physical_device, m_surface);

  vk::CommandPoolCreateInfo ci;
  ci.queueFamilyIndex = indices.graphics_family.value();
  ci.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

  try {
    m_graphic_command_pool = m_device.createCommandPool(ci);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create graphics command pool: ") + e.what());
  }
}

void Context::_create_swap_chain(const Window &window) {
  m_swap_chain = std::make_unique<SwapChain>(this, window);
}

void Context::_allocate_command_buffers() {
  // Create command buffer for graphics commands
  vk::CommandBufferAllocateInfo ai;
  ai.commandPool = m_graphic_command_pool;
  ai.level = vk::CommandBufferLevel::ePrimary;
  ai.commandBufferCount =
      static_cast<uint32_t>(m_swap_chain->get_image_count());

  try {
    m_graphic_command_buffers = m_device.allocateCommandBuffers(ai);
  } catch (const std::runtime_error &e) {
    throw VulkanKraftException(
        std::string("failed to create graphic command buffer: ") + e.what());
  }
}

void Context::_create_sync_objects() {
  m_image_available_semaphores.resize(_max_images_in_flight);
  m_render_finished_semaphores.resize(_max_images_in_flight);
  m_in_flight_fences.resize(_max_images_in_flight);
  m_images_in_flight.resize(m_swap_chain->get_image_count(), VK_NULL_HANDLE);

  vk::SemaphoreCreateInfo si;

  vk::FenceCreateInfo fi;
  fi.flags = vk::FenceCreateFlagBits::eSignaled;

  for (size_t i = 0; i < _max_images_in_flight; i++) {
    try {
      m_image_available_semaphores[i] = m_device.createSemaphore(si);
      m_render_finished_semaphores[i] = m_device.createSemaphore(si);
      m_in_flight_fences[i] = m_device.createFence(fi);
    } catch (const std::runtime_error &e) {
      throw VulkanKraftException(
          "failed to create synchronisation objects for frame " +
          std::to_string(i) + ": " + e.what());
    }
  }
}

void Context::_handle_framebuffer_resize() {
  m_device.waitIdle();

  m_swap_chain->recreate();
}

} // namespace vulkan
} // namespace core