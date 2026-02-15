package net.zzcjas.nuclearindustry;

import net.minecraft.core.BlockPos;
import net.minecraft.server.level.ServerLevel;
import net.minecraft.world.level.block.Blocks;
import net.minecraft.world.level.block.state.BlockState;

public class BlockExplosionDefense {

    /**
     * 获取方块的防御值（基于原版爆炸抗性）
     * - 基岩等不可破坏方块返回极大值 (10_000)
     * - 流体返回 0.2（确保被摧毁）
     * - 可替换/无碰撞方块返回 0.1（草、花等）
     * - 其他方块返回其爆炸抗性（已缩放至合理范围，可直接用于射线穿透计算）
     */
    public static float getBlockDefenseValue(ServerLevel level, BlockPos pos, BlockState state) {
        if (state.isAir()) return 0.0F;

        // 绝对防御方块（基岩、命令方块等）
        if (state.is(Blocks.BEDROCK) ||
            state.is(Blocks.COMMAND_BLOCK) ||
            state.is(Blocks.END_PORTAL_FRAME) ||
            state.is(Blocks.END_GATEWAY) ||
            state.is(Blocks.STRUCTURE_BLOCK) ||
            state.is(Blocks.JIGSAW)) {
            return 10_000.0F;
        }

        // 流体
        if (!state.getFluidState().isEmpty()) {
            return 0.2F;
        }

        // 可替换方块（草、花、雪层等）或空碰撞
        if (state.canBeReplaced() || state.getCollisionShape(level, pos).isEmpty()) {
            return 0.1F;
        }

        // 直接返回原版爆炸抗性（已符合大多数方块的相对强度）
        return (int)(state.getBlock().getExplosionResistance());
    }
}