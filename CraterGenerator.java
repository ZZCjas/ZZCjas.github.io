package net.zzcjas.nuclearindustry;

import net.minecraft.core.BlockPos;
import net.minecraft.server.MinecraftServer;
import net.minecraft.server.TickTask;
import net.minecraft.server.level.ServerLevel;
import net.minecraft.tags.BlockTags;
import net.minecraft.util.RandomSource;
import net.minecraft.world.entity.LivingEntity;
import net.minecraft.world.entity.item.ItemEntity;
import net.minecraft.world.level.ClipContext;
import net.minecraft.world.level.block.Block;
import net.minecraft.world.level.block.Blocks;
import net.minecraft.world.level.block.state.BlockState;
import net.minecraft.world.phys.AABB;
import net.minecraft.world.phys.BlockHitResult;
import net.minecraft.world.phys.HitResult;
import net.minecraft.world.phys.Vec3;
import net.minecraftforge.server.ServerLifecycleHooks;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class CraterGenerator {

    // ==========================================
    // 性能与生成设置
    // ==========================================
    private static final int CRATER_GENERATION_DELAY = 30;   // 延迟启动（tick）
    private static final int BLOCK_BATCH_SIZE = 512;         // 每 tick 处理的方块数（最终阶段）
    private static final int RAYS_PER_TICK = 500;            // 每 tick 计算的射线数（形状计算阶段）
    private static final int DAMAGE_CHUNKS_PER_TICK = 16;    // 每 tick 处理的区块数（实体伤害阶段）

    // ==========================================
    // 射线追踪设置
    // ==========================================
    private static final int TOTAL_RAYS = 103680;            // 总射线数（决定细节）
    private static final int RING_COUNT = 8;                 // 环数（用于排序）
    private static final int RAY_THICKNESS = 3;              // 射线厚度（平滑用）
    private static final double MAX_RAY_DISTANCE = 100.0;    // 最大射线距离（弹坑半径）
    private static final double MAX_PENETRATION = 10000.0;   // 最大穿透力（可破坏黑曜石）
    private static final double MIN_PENETRATION = 2000.0;    // 最小穿透力
    public static long RAY_TRAIL_DURATION = 60_000L;         // 调试射线存活时间（ms）

    // ==========================================
    // 伤害区设置
    // ==========================================
    private static final float ZONE_3_RADIUS_MULTIPLIER = 0.9F;
    private static final float ZONE_4_RADIUS_MULTIPLIER = 1.8F;
    private static final float ZONE_3_DAMAGE = 5000.0F;      // 内圈伤害
    private static final float ZONE_4_DAMAGE = 2000.0F;      // 外圈伤害
    private static final float FIRE_DURATION = 380.0F;       // 着火时间（tick）
    private static final int DAMAGE_ZONE_HEIGHT = 80;        // 伤害垂直范围

    // ==========================================
    // 噪声与边界设置
    // ==========================================
    private static final int ZONE_OVERLAP = 15;
    private static final double ZONE_NOISE_SCALE = 0.15;
    private static final double ZONE_NOISE_STRENGTH = 0.25;
    private static final float ZONE_GRADIENT_THICKNESS = 1.15F;

    // ==========================================
    // 其他
    // ==========================================
    private static final int CENTER_SPHERE_RADIUS = 5;
    private static final float CLEANUP_DEFENSE_THRESHOLD = 1500.0F;

    private static final Map<BlockPos, List<RayData>> rayDebugData = new ConcurrentHashMap<>();

    public static class RayData {
        public final double startX, startY, startZ;
        public final double dirX, dirY, dirZ;
        public final List<BlockPos> hitBlocks;
        public final long timestamp;
        public final long expirationTime;

        RayData(double startX, double startY, double startZ,
                double dirX, double dirY, double dirZ) {
            this.startX = startX;
            this.startY = startY;
            this.startZ = startZ;
            this.dirX = dirX;
            this.dirY = dirY;
            this.dirZ = dirZ;
            this.hitBlocks = Collections.synchronizedList(new ArrayList<>());
            this.timestamp = System.currentTimeMillis();
            this.expirationTime = timestamp + RAY_TRAIL_DURATION;
        }

        public boolean isExpired() {
            return System.currentTimeMillis() > expirationTime;
        }

        public float getRemainingSeconds() {
            long remaining = expirationTime - System.currentTimeMillis();
            return Math.max(0, remaining / 1000.0f);
        }
    }

    private static class RayTerminationData {
        double maxDistance = 0;
        Set<BlockPos> terminationPoints = Collections.synchronizedSet(new HashSet<>());
    }

    /**
     * 生成核爆弹坑（完全移植，无 HBM 特有方块）
     * @param level             世界
     * @param centerPos         中心坐标
     * @param craterFillBlock   弹坑内部填充方块（如石头、岩浆等，传入原版方块）
     * @param wasteLogBlock     焦黑原木（如 Blocks.OAK_LOG）
     * @param wastePlanksBlock  焦黑木板
     * @param burnedGrassBlock  烧焦草方块
     * @param deadDirtBlock     死土
     */
    public static void generateCrater(
            ServerLevel level,
            BlockPos centerPos,
            Block craterFillBlock,
            Block wasteLogBlock,
            Block wastePlanksBlock,
            Block burnedGrassBlock,
            Block deadDirtBlock) {

        BlockPos groundCenterPos = centerPos;
        RandomSource random = level.random;
        Set<BlockPos> craterBlocksSet = Collections.synchronizedSet(new HashSet<>());
        List<Set<BlockPos>> rings = new ArrayList<>();
        RayTerminationData terminationData = new RayTerminationData();
        rayDebugData.clear();
        List<RayData> currentRayDebugList = Collections.synchronizedList(new ArrayList<>());

        for (int i = 0; i < RING_COUNT; i++) {
            rings.add(Collections.synchronizedSet(new HashSet<>()));
        }

        long startTime = System.currentTimeMillis();
        System.out.println("\n========================================");
        System.out.println("[CRATER_GENERATOR] START: Generating crater...");
        System.out.println("Bomb Position: " + centerPos);
        System.out.println("Total Rays: " + TOTAL_RAYS);
        System.out.println("========================================\n");

        BlockPos below = groundCenterPos.below();
        BlockState belowState = level.getBlockState(below);

        if (!belowState.isAir() && !belowState.is(Blocks.BEDROCK)) {
            craterBlocksSet.add(below);
            distributeBlockToRings(groundCenterPos, below, (int) MAX_RAY_DISTANCE, rings);
        }

        collectSphericRaysAsync(level, groundCenterPos, craterBlocksSet, rings, terminationData, currentRayDebugList, () -> {
            System.out.println("\n[CRATER_GENERATOR] Step 1: Finalizing crater structure...");
            if (!currentRayDebugList.isEmpty()) {
                rayDebugData.put(groundCenterPos, new ArrayList<>(currentRayDebugList));
                System.out.println("[DEBUG] Stored " + currentRayDebugList.size() + " rays for visualization");
            }

            finalizeCrater(level, groundCenterPos, rings, craterBlocksSet,
                    craterFillBlock, random,
                    wasteLogBlock, wastePlanksBlock, burnedGrassBlock, startTime);

            MinecraftServer server = ServerLifecycleHooks.getCurrentServer();
            if (server != null) {
                server.tell(new TickTask(5, () -> {
                    System.out.println("\n[CRATER_GENERATOR] Step 1.5: Cleaning center sphere...");
                    cleanupCenterSphere(level, groundCenterPos);

                    server.tell(new TickTask(5, () -> {
                        System.out.println("\n[CRATER_GENERATOR] Step 2: Calculating and applying dynamic damage zones SEQUENTIALLY...");

                        double zone3Radius = Math.max(terminationData.maxDistance * ZONE_3_RADIUS_MULTIPLIER, 50);
                        double zone4Radius = Math.max(terminationData.maxDistance * ZONE_4_RADIUS_MULTIPLIER, 80);

                        System.out.println("[CRATER_GENERATOR] Starting Zone Sequence...");

                        processZone3(level, groundCenterPos, zone3Radius, zone4Radius,
                                wasteLogBlock, wastePlanksBlock, burnedGrassBlock, deadDirtBlock, craterFillBlock, random);
                    }));
                }));
            }
        });
    }

    public static List<RayData> getDebugRays(BlockPos centerPos) {
        List<RayData> rays = rayDebugData.getOrDefault(centerPos, new ArrayList<>());
        rays.removeIf(RayData::isExpired);
        return rays;
    }

    public static List<RayData> getAllDebugRays() {
        List<RayData> allRays = new ArrayList<>();
        for (List<RayData> rayList : rayDebugData.values()) {
            allRays.addAll(rayList);
        }
        allRays.removeIf(RayData::isExpired);
        return allRays;
    }

    // ========== 噪声辅助函数 ==========
    private static double getSimpleWaveNoise(double x, double z) {
        double wave1 = Math.sin(x * 0.1) * Math.cos(z * 0.1) * 0.5;
        double wave2 = Math.sin(x * 0.05 + z * 0.08) * 0.3;
        double wave3 = Math.cos(x * 0.15 - z * 0.12) * 0.2;
        return (wave1 + wave2 + wave3) / 2.0;
    }

    private static double getZoneRadiusWithNoise(double baseRadius, double centerX, double centerZ, int x, int z) {
        double relX = (x - centerX) * ZONE_NOISE_SCALE;
        double relZ = (z - centerZ) * ZONE_NOISE_SCALE;
        double noise = getSimpleWaveNoise(relX, relZ);
        double noiseInfluence = 1.0 + (noise * ZONE_NOISE_STRENGTH);
        return baseRadius * noiseInfluence;
    }

    private static void cleanupCenterSphere(ServerLevel level, BlockPos centerPos) {
        int centerX = centerPos.getX();
        int centerY = centerPos.getY();
        int centerZ = centerPos.getZ();
        int removed = 0;

        for (int x = centerX - CENTER_SPHERE_RADIUS; x <= centerX + CENTER_SPHERE_RADIUS; x++) {
            long dx = x - centerX;
            long dxSq = dx * dx;
            if (dxSq > CENTER_SPHERE_RADIUS * CENTER_SPHERE_RADIUS) continue;

            for (int z = centerZ - CENTER_SPHERE_RADIUS; z <= centerZ + CENTER_SPHERE_RADIUS; z++) {
                long dz = z - centerZ;
                long distSq = dxSq + dz * dz;
                if (distSq > CENTER_SPHERE_RADIUS * CENTER_SPHERE_RADIUS) continue;

                for (int y = centerY - CENTER_SPHERE_RADIUS; y <= centerY + CENTER_SPHERE_RADIUS; y++) {
                    long dy = y - centerY;
                    long totalDistSq = distSq + dy * dy;
                    if (totalDistSq > CENTER_SPHERE_RADIUS * CENTER_SPHERE_RADIUS) continue;

                    BlockPos checkPos = new BlockPos(x, y, z);
                    BlockState state = level.getBlockState(checkPos);

                    if (!state.isAir() && !state.is(Blocks.BEDROCK)) {
                        float defense = BlockExplosionDefense.getBlockDefenseValue(level, checkPos, state);
                        if (defense < CLEANUP_DEFENSE_THRESHOLD) {
                            level.removeBlock(checkPos, false);
                            removed++;
                        }
                    }
                }
            }
        }
        System.out.println("[CRATER] Center sphere cleanup: " + removed + " blocks removed");
    }

    private static double calculatePenetrationFromAngle(double verticalAngleDegrees) {
        double absAngle = Math.abs(verticalAngleDegrees);
        if (absAngle > 90.0) absAngle = 90.0;
        double angleRatio = absAngle / 90.0;
        double penetrationFactor = 1.0 - (angleRatio * angleRatio * 0.60);
        double penetration = MIN_PENETRATION + (MAX_PENETRATION - MIN_PENETRATION) * penetrationFactor;
        return Math.max(MIN_PENETRATION, Math.min(MAX_PENETRATION, penetration));
    }

    private static void collectSphericRaysAsync(
            ServerLevel level,
            BlockPos centerPos,
            Set<BlockPos> craterBlocksSet,
            List<Set<BlockPos>> rings,
            RayTerminationData terminationData,
            List<RayData> debugRays,
            Runnable onComplete) {

        MinecraftServer server = ServerLifecycleHooks.getCurrentServer();
        if (server == null) {
            onComplete.run();
            return;
        }

        int[] rayIndex = new int[1];
        rayIndex[0] = 0;
        processRaysBatchSpheric(level, centerPos, craterBlocksSet, rings, terminationData,
                rayIndex, server, debugRays, onComplete);
    }

    private static void processRaysBatchSpheric(
            ServerLevel level,
            BlockPos centerPos,
            Set<BlockPos> craterBlocksSet,
            List<Set<BlockPos>> rings,
            RayTerminationData terminationData,
            int[] currentRayIndex,
            MinecraftServer server,
            List<RayData> debugRays,
            Runnable onComplete) {

        double startX = centerPos.getX() + 0.5;
        double startY = centerPos.getY() + 0.5;
        double startZ = centerPos.getZ() + 0.5;

        int raysToProcess = Math.min(RAYS_PER_TICK, TOTAL_RAYS - currentRayIndex[0]);
        int processed = 0;

        while (processed < raysToProcess && currentRayIndex[0] < TOTAL_RAYS) {
            int rayIndex = currentRayIndex[0];
            double phi = Math.PI * (3.0 - Math.sqrt(5.0));
            double theta = Math.acos(1.0 - (2.0 * rayIndex) / TOTAL_RAYS);
            double psi = phi * rayIndex;

            double sinTheta = Math.sin(theta);
            double cosTheta = Math.cos(theta);
            double cosPsi = Math.cos(psi);
            double sinPsi = Math.sin(psi);

            double dirX = sinTheta * cosPsi;
            double dirY = cosTheta;
            double dirZ = sinTheta * sinPsi;

            double elevationDegrees = Math.toDegrees(Math.asin(dirY));
            if (elevationDegrees < 0) {
                elevationDegrees *= 0.85;
            }

            double basePenetration = calculatePenetrationFromAngle(elevationDegrees);

            double noiseScale = 0.08;
            double noiseStrength = 0.35;

            int maxStepsModified = CraterNoiseGenerator.getNoiseModifiedMaxDistance(
                    (int)MAX_RAY_DISTANCE,
                    dirX, dirY, dirZ,
                    noiseScale,
                    noiseStrength
            );

            RayData debugRay = null;
            if (isDebugScreenEnabled()) {
                debugRay = new RayData(startX, startY, startZ, dirX, dirY, dirZ);
            }

            traceRay(level, startX, startY, startZ, dirX, dirY, dirZ, basePenetration,
                    maxStepsModified,
                    centerPos, craterBlocksSet, rings, terminationData, debugRay);

            if (debugRay != null && !debugRay.hitBlocks.isEmpty()) {
                debugRays.add(debugRay);
            }

            currentRayIndex[0]++;
            processed++;
        }

        if (currentRayIndex[0] < TOTAL_RAYS) {
            server.tell(new TickTask(1, () ->
                    processRaysBatchSpheric(level, centerPos, craterBlocksSet, rings, terminationData,
                            currentRayIndex, server, debugRays, onComplete)
            ));
        } else {
            onComplete.run();
        }
    }

    private static int floorCoordinate(double coord) {
        return (int) Math.floor(coord);
    }

    private static void traceRay(
            ServerLevel level,
            double startX, double startY, double startZ,
            double dirX, double dirY, double dirZ,
            double basePenetration,
            int maxSteps,
            BlockPos centerPos,
            Set<BlockPos> craterBlocksSet,
            List<Set<BlockPos>> rings,
            RayTerminationData terminationData,
            RayData debugRay) {

        double penetration = basePenetration;
        if (penetration < MIN_PENETRATION || penetration > MAX_PENETRATION) {
            return;
        }

        BlockPos lastBlockPos = centerPos;
        double perpX1, perpY1, perpZ1;
        double perpX2, perpY2, perpZ2;

        double absX = Math.abs(dirX);
        double absY = Math.abs(dirY);
        double absZ = Math.abs(dirZ);

        if (absZ <= absX && absZ <= absY) {
            perpX1 = dirY;
            perpY1 = -dirX;
            perpZ1 = 0;
        } else if (absY <= absX && absY <= absZ) {
            perpX1 = dirZ;
            perpY1 = 0;
            perpZ1 = -dirX;
        } else {
            perpX1 = 0;
            perpY1 = dirZ;
            perpZ1 = -dirY;
        }

        double len1 = Math.sqrt(perpX1 * perpX1 + perpY1 * perpY1 + perpZ1 * perpZ1);
        if (len1 > 0) {
            perpX1 /= len1;
            perpY1 /= len1;
            perpZ1 /= len1;
        }

        perpX2 = dirY * perpZ1 - dirZ * perpY1;
        perpY2 = dirZ * perpX1 - dirX * perpZ1;
        perpZ2 = dirX * perpY1 - dirY * perpX1;

        double len2 = Math.sqrt(perpX2 * perpX2 + perpY2 * perpY2 + perpZ2 * perpZ2);
        if (len2 > 0) {
            perpX2 /= len2;
            perpY2 /= len2;
            perpZ2 /= len2;
        }

        for (int step = 1; step <= maxSteps && penetration > 0; step++) {
            int baseX = floorCoordinate(startX + dirX * step);
            int baseY = floorCoordinate(startY + dirY * step);
            int baseZ = floorCoordinate(startZ + dirZ * step);

            boolean blockedAtStep = false;

            double defenseCenter = processBlockInRay(level, baseX, baseY, baseZ,
                    centerPos, craterBlocksSet, rings);

            if (defenseCenter > 0) {
                lastBlockPos = new BlockPos(baseX, baseY, baseZ);
                if (debugRay != null) {
                    debugRay.hitBlocks.add(lastBlockPos);
                }

                penetration -= defenseCenter;
                if (penetration < 0) penetration = 0;

                if (defenseCenter >= 10_000) {
                    blockedAtStep = true;
                }
            }

            if (!blockedAtStep) {
                int[] thicknessOffsets = {1, -1, 2, -2, 3, -3};
                for (int thickness : thicknessOffsets) {
                    if (blockedAtStep) break;

                    int x1 = baseX + (int) Math.round(perpX1 * thickness);
                    int y1 = baseY + (int) Math.round(perpY1 * thickness);
                    int z1 = baseZ + (int) Math.round(perpZ1 * thickness);

                    double defense1 = processBlockInRay(level, x1, y1, z1, centerPos, craterBlocksSet, rings);
                    penetration -= defense1;
                    if (penetration < 0) penetration = 0;
                    if (defense1 >= 10_000) blockedAtStep = true;

                    int x2 = baseX + (int) Math.round(perpX2 * thickness);
                    int y2 = baseY + (int) Math.round(perpY2 * thickness);
                    int z2 = baseZ + (int) Math.round(perpZ2 * thickness);

                    double defense2 = processBlockInRay(level, x2, y2, z2, centerPos, craterBlocksSet, rings);
                    penetration -= defense2;
                    if (penetration < 0) penetration = 0;
                    if (defense2 >= 10_000) blockedAtStep = true;
                }
            }
        }

        if (lastBlockPos != centerPos) {
            terminationData.terminationPoints.add(lastBlockPos);
            double distance = Math.sqrt(
                    Math.pow(lastBlockPos.getX() - centerPos.getX(), 2) +
                            Math.pow(lastBlockPos.getZ() - centerPos.getZ(), 2)
            );

            synchronized (terminationData) {
                if (distance > terminationData.maxDistance) {
                    terminationData.maxDistance = distance;
                }
            }
        }
    }

    private static double processBlockInRay(
            ServerLevel level,
            int x, int y, int z,
            BlockPos centerPos,
            Set<BlockPos> craterBlocksSet,
            List<Set<BlockPos>> rings) {

        BlockPos pos = new BlockPos(x, y, z);
        BlockState state = level.getBlockState(pos);

        if (state.isAir()) {
            return 0;
        }

        if (!state.getFluidState().isEmpty()) {
            synchronized (craterBlocksSet) {
                if (!craterBlocksSet.contains(pos)) {
                    craterBlocksSet.add(pos);
                    distributeBlockToRings(centerPos, pos, (int) MAX_RAY_DISTANCE, rings);
                }
            }
            return 0.2;
        }

        if (state.canBeReplaced() || state.getCollisionShape(level, pos).isEmpty()) {
            synchronized (craterBlocksSet) {
                if (!craterBlocksSet.contains(pos)) {
                    craterBlocksSet.add(pos);
                    distributeBlockToRings(centerPos, pos, (int) MAX_RAY_DISTANCE, rings);
                }
            }
            return 0.1;
        }

        float defense = BlockExplosionDefense.getBlockDefenseValue(level, pos, state);

        if (defense < 10_000) {
            synchronized (craterBlocksSet) {
                if (!craterBlocksSet.contains(pos)) {
                    craterBlocksSet.add(pos);
                    distributeBlockToRings(centerPos, pos, (int) MAX_RAY_DISTANCE, rings);
                }
            }
            return defense;
        }

        return defense;
    }

    private static void distributeBlockToRings(
            BlockPos center, BlockPos pos, int maxRadius, List<Set<BlockPos>> rings) {

        double distSq = center.distSqr(pos);
        double maxDistSq = (double) maxRadius * maxRadius;
        double ratio = Math.min(distSq / maxDistSq, 1.0);
        int ringIndex = (int) (ratio * RING_COUNT);

        if (ringIndex < 0) ringIndex = 0;
        if (ringIndex >= RING_COUNT) ringIndex = RING_COUNT - 1;

        rings.get(ringIndex).add(pos);
    }

    private static void finalizeCrater(
            ServerLevel level,
            BlockPos centerPos,
            List<Set<BlockPos>> rings,
            Set<BlockPos> allCraterBlocks,
            Block craterFillBlock,
            RandomSource random,
            Block wasteLog,
            Block wastePlanks,
            Block burnedGrass,
            long startTime) {

        processAllRingsBatched(level, centerPos, rings, allCraterBlocks,
                craterFillBlock, random, wasteLog, wastePlanks, burnedGrass);

        removeItemsInRadiusBatched(level, centerPos, (int) MAX_RAY_DISTANCE + 20);

        long endTime = System.currentTimeMillis();
        System.out.println("[CRATER] Generation complete! Time: " +
                (endTime - startTime) + "ms | Total Rays: " + TOTAL_RAYS);
    }

    private static void processAllRingsBatched(
            ServerLevel level,
            BlockPos centerPos,
            List<Set<BlockPos>> rings,
            Set<BlockPos> allCraterBlocks,
            Block craterFillBlock,
            RandomSource random,
            Block wasteLog,
            Block wastePlanks,
            Block burnedGrass) {

        MinecraftServer server = ServerLifecycleHooks.getCurrentServer();
        if (server == null) return;

        List<BlockPos> allBlocksList = new ArrayList<>(allCraterBlocks);
        int totalBlocks = allBlocksList.size();
        int totalBatches = (int) Math.ceil((double) totalBlocks / BLOCK_BATCH_SIZE);

        for (int i = 0; i < totalBatches; i++) {
            final int batchIndex = i;
            server.tell(new TickTask(server.getTickCount() + i + 1, () -> {
                int start = batchIndex * BLOCK_BATCH_SIZE;
                int end = Math.min(start + BLOCK_BATCH_SIZE, totalBlocks);

                for (int j = start; j < end; j++) {
                    BlockPos pos = allBlocksList.get(j);
                    boolean isBorder = false;

                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dy = -1; dy <= 1; dy++) {
                            for (int dz = -1; dz <= 1; dz++) {
                                if (Math.abs(dx) + Math.abs(dy) + Math.abs(dz) == 1) {
                                    BlockPos neighbor = pos.offset(dx, dy, dz);
                                    BlockState neighborState = level.getBlockState(neighbor);

                                    if (!allCraterBlocks.contains(neighbor) &&
                                            !neighborState.isAir() &&
                                            !neighborState.getCollisionShape(level, neighbor).isEmpty()) {
                                        isBorder = true;
                                        break;
                                    }
                                }
                            }
                            if (isBorder) break;
                        }
                        if (isBorder) break;
                    }

                    if (isBorder) {
                        BlockState state = level.getBlockState(pos);

                        if (state.is(BlockTags.LEAVES) || state.is(BlockTags.FLOWERS) ||
                                state.is(Blocks.GRASS) || state.is(Blocks.TALL_GRASS) ||
                                state.getCollisionShape(level, pos).isEmpty()) {
                            level.removeBlock(pos, false);
                        } else {
                            level.setBlock(pos, craterFillBlock.defaultBlockState(), 3);
                        }
                    } else {
                        level.removeBlock(pos, false);
                    }
                }
            }));
        }
    }

    private static void removeItemsInRadiusBatched(ServerLevel level, BlockPos center, int radius) {
        AABB box = new AABB(center).inflate(radius);
        List<ItemEntity> items = level.getEntitiesOfClass(ItemEntity.class, box);
        for (ItemEntity item : items) {
            item.discard();
        }
    }

    // ========== 区域处理链 ==========
    private static void processZone3(ServerLevel level, BlockPos centerPos, double r3, double r4,
                                     Block wasteLog, Block wastePlanks, Block burnedGrass, Block deadDirt,
                                     Block craterFill, RandomSource random) {
        int startScan = 0;
        int endScan = (int) Math.ceil(r3) + ZONE_OVERLAP;

        processZoneBatch(level, centerPos, startScan, endScan, r3, r4, 3,
                wasteLog, wastePlanks, burnedGrass, deadDirt, craterFill, random, () -> {
                    MinecraftServer server = level.getServer();
                    if (server != null) {
                        server.tell(new TickTask(server.getTickCount() + 2, () ->
                                processZone4(level, centerPos, r3, r4, wasteLog, wastePlanks, burnedGrass, deadDirt, craterFill, random)
                        ));
                    }
                });
    }

    private static void processZone4(ServerLevel level, BlockPos centerPos, double r3, double r4,
                                     Block wasteLog, Block wastePlanks, Block burnedGrass, Block deadDirt,
                                     Block craterFill, RandomSource random) {
        int startScan = Math.max(0, (int) Math.floor(r3) - ZONE_OVERLAP);
        int endScan = (int) Math.ceil(r4) + ZONE_OVERLAP;

        processZoneBatch(level, centerPos, startScan, endScan, r3, r4, 4,
                wasteLog, wastePlanks, burnedGrass, deadDirt, craterFill, random, () -> {
                    MinecraftServer server = level.getServer();
                    if (server != null) {
                        server.tell(new TickTask(server.getTickCount() + 2, () ->
                                processZone5(level, centerPos, r3, r4, wasteLog, wastePlanks, burnedGrass, deadDirt, craterFill, random)
                        ));
                    }
                });
    }

    private static void processZone5(ServerLevel level, BlockPos centerPos, double r3, double r4,
                                     Block wasteLog, Block wastePlanks, Block burnedGrass, Block deadDirt,
                                     Block craterFill, RandomSource random) {
        double r5 = r4 + 12.0;
        int startScan = Math.max(0, (int) Math.floor(r4) - ZONE_OVERLAP);
        int endScan = (int) Math.ceil(r5) + ZONE_OVERLAP;

        processZoneBatch(level, centerPos, startScan, endScan, r3, r4, 5,
                wasteLog, wastePlanks, burnedGrass, deadDirt, craterFill, random, () -> {
                    MinecraftServer server = level.getServer();
                    if (server != null) {
                        server.tell(new TickTask(server.getTickCount() + 2, () ->
                                processZone6(level, centerPos, r3, r4, wasteLog, wastePlanks, burnedGrass, deadDirt, craterFill, random)
                        ));
                    }
                });
    }

    private static void processZone6(ServerLevel level, BlockPos centerPos, double r3, double r4,
                                     Block wasteLog, Block wastePlanks, Block burnedGrass, Block deadDirt,
                                     Block craterFill, RandomSource random) {
        double r5 = r4 + 12.0;
        double r6 = r4 + 24.0;
        int startScan = Math.max(0, (int) Math.floor(r5) - ZONE_OVERLAP);
        int endScan = (int) Math.ceil(r6) + ZONE_OVERLAP;

        processZoneBatch(level, centerPos, startScan, endScan, r3, r4, 6,
                wasteLog, wastePlanks, burnedGrass, deadDirt, craterFill, random, () -> {

                    MinecraftServer server = level.getServer();
                    if (server != null) {
                        server.tell(new TickTask(server.getTickCount() + 1, () -> {
                            System.out.println("\n[CRATER_GENERATOR] Step 3: Finalizing...");
                            // 原 HBM 调用了 CraterBiomeHelper，此处移除或可自行实现
                            // CraterBiomeHelper.applyBiomesAsync(level, centerPos, r3, r4);
                            applyDamageToEntities(level, centerPos, r3, r4, random);
                            cleanupItems(level, centerPos, r3 + 10);
                            System.out.println("\n[CRATER_GENERATOR] All steps complete!");
                        }));
                    }
                });
    }

    private static void processZoneBatch(
            ServerLevel level, BlockPos center, int startRadius, int endRadius,
            double r3Base, double r4Base, int zoneType,
            Block wasteLog, Block wastePlanks, Block burnedGrass, Block deadDirt, Block craterFill,
            RandomSource random, Runnable onComplete) {

        MinecraftServer server = level.getServer();
        if (server == null) return;

        int[] currentOffset = {0};
        processZoneBatchStep(level, center, startRadius, endRadius, currentOffset, 4,
                r3Base, r4Base, zoneType, wasteLog, wastePlanks, burnedGrass, deadDirt, craterFill, random, server, onComplete);
    }

    private static void processZoneBatchStep(
            ServerLevel level, BlockPos center, int startRadius, int endRadius,
            int[] offsetRef, int stepSize,
            double r3Base, double r4Base, int zoneType,
            Block wasteLog, Block wastePlanks, Block burnedGrass, Block deadDirt, Block craterFill,
            RandomSource random, MinecraftServer server, Runnable onComplete) {

        int currentStart = startRadius + offsetRef[0];
        int currentEnd = Math.min(currentStart + stepSize, endRadius);

        int centerX = center.getX();
        int centerZ = center.getZ();

        for (int x = centerX - currentEnd; x <= centerX + currentEnd; x++) {
            for (int z = centerZ - currentEnd; z <= centerZ + currentEnd; z++) {

                double dx = x - centerX;
                double dz = z - centerZ;
                double distSq = dx * dx + dz * dz;
                double dist = Math.sqrt(distSq);

                if (dist < currentStart || dist > currentEnd) continue;

                double r3Noise = getZoneRadiusWithNoise(r3Base, centerX, centerZ, x, z);
                double r4Noise = getZoneRadiusWithNoise(r4Base, centerX, centerZ, x, z);
                double r5Noise = getZoneRadiusWithNoise(r4Base + 12.0, centerX, centerZ, x, z);
                double r6Noise = getZoneRadiusWithNoise(r4Base + 24.0, centerX, centerZ, x, z);

                boolean inZone = false;

                if (zoneType == 3) {
                    inZone = dist <= r3Noise;
                } else if (zoneType == 4) {
                    inZone = dist > r3Noise && dist <= r4Noise;
                } else if (zoneType == 5) {
                    inZone = dist > r4Noise && dist <= r5Noise;
                } else if (zoneType == 6) {
                    inZone = dist > r5Noise && dist <= r6Noise;
                }

                if (!inZone) continue;

                BlockPos.MutableBlockPos mutablePos = new BlockPos.MutableBlockPos();
                for (int y = center.getY() + 80; y >= center.getY() - 64; y--) {
                    mutablePos.set(x, y, z);
                    BlockState state = level.getBlockState(mutablePos);
                    if (state.isAir()) continue;

                    BlockPos fixedPos = mutablePos.immutable();

                    if (zoneType == 3) {
                        if (transformBlockInZone3(level, fixedPos, state, craterFill, random, wasteLog, wastePlanks)) {
                            break;
                        }
                    } else if (zoneType == 4) {
                        applyZone4Effects(level, fixedPos, state, wasteLog, wastePlanks, burnedGrass, deadDirt, random);
                    } else if (zoneType == 5) {
                        applyZone5Effects(level, fixedPos, state, wasteLog, wastePlanks, random);
                    } else if (zoneType == 6) {
                        applyZone6Effects(level, fixedPos, state, random);
                    }

                    if (zoneType >= 4) {
                        BlockState newState = level.getBlockState(fixedPos);
                        if (!newState.isAir()) {
                            tryApplyNuclearFallout(level, fixedPos.above(), newState, random);
                        }
                    }
                }
            }
        }

        offsetRef[0] += stepSize;

        if (offsetRef[0] < (endRadius - startRadius)) {
            server.tell(new TickTask(server.getTickCount() + 1, () ->
                    processZoneBatchStep(level, center, startRadius, endRadius, offsetRef, stepSize,
                            r3Base, r4Base, zoneType, wasteLog, wastePlanks, burnedGrass, deadDirt, craterFill, random, server, onComplete)
            ));
        } else {
            onComplete.run();
        }
    }

    private static boolean transformBlockInZone3(ServerLevel level, BlockPos pos, BlockState state,
                                                 Block craterFill, RandomSource random,
                                                 Block wasteLog, Block wastePlanks) {
        if (state.is(Blocks.BEDROCK)) return true;

        if (state.is(BlockTags.LOGS)) {
            level.setBlock(pos, wasteLog.defaultBlockState(), 3);
            return false;
        }

        if (state.is(BlockTags.PLANKS)) {
            level.setBlock(pos, wastePlanks.defaultBlockState(), 3);
            return false;
        }

        boolean isOtherWood = state.getSoundType() == net.minecraft.world.level.block.SoundType.WOOD;
        boolean isFoliage = state.is(BlockTags.LEAVES) || state.is(BlockTags.FLOWERS) || state.is(Blocks.GRASS) || state.is(Blocks.TALL_GRASS) || state.is(Blocks.VINE);
        boolean isWeak = state.getCollisionShape(level, pos).isEmpty() || state.is(Blocks.SNOW) || state.is(Blocks.SNOW_BLOCK) || state.is(BlockTags.WOOL);

        if (isFoliage || isWeak || isOtherWood) {
            level.removeBlock(pos, false);
            return false;
        }

        if (!Block.isFaceFull(state.getCollisionShape(level, pos), net.minecraft.core.Direction.UP)) {
            level.removeBlock(pos, false);
            return false;
        }

        level.setBlock(pos, craterFill.defaultBlockState(), 3);
        return true;
    }

    private static void applyZone4Effects(
            ServerLevel level,
            BlockPos pos,
            BlockState state,
            Block wasteLogBlock,
            Block wastePlanksBlock,
            Block burnedGrassBlock,
            Block deadDirtBlock,
            RandomSource random) {

        if (state.isAir()) return;
        if (state.is(Blocks.BEDROCK)) return;

        if (state.is(BlockTags.LOGS)) {
            level.setBlock(pos, wasteLogBlock.defaultBlockState(), 3);
        } else if (state.is(BlockTags.PLANKS)) {
            level.setBlock(pos, wastePlanksBlock.defaultBlockState(), 3);
        } else if (state.is(BlockTags.WOODEN_STAIRS) || state.is(BlockTags.WOODEN_SLABS) || state.is(BlockTags.LEAVES) ||
                state.is(BlockTags.WOODEN_TRAPDOORS) || state.is(Blocks.TORCH) || state.is(BlockTags.WOOL_CARPETS) ||
                state.is(BlockTags.WOOL) || state.is(BlockTags.WOODEN_FENCES) || state.is(Blocks.PUMPKIN) ||
                state.is(BlockTags.WOODEN_DOORS)) {
            level.removeBlock(pos, false);
        } else if (state.is(Blocks.GRASS_BLOCK) || state.is(Blocks.DIRT_PATH) ||
                state.is(Blocks.MYCELIUM) || state.is(Blocks.PODZOL)) {
            level.setBlock(pos, burnedGrassBlock.defaultBlockState(), 3);
        } else if (state.is(Blocks.DIRT) || state.is(Blocks.COARSE_DIRT) ||
                state.is(Blocks.ROOTED_DIRT)) {
            level.setBlock(pos, deadDirtBlock.defaultBlockState(), 3);
        } else if (state.is(Blocks.GRASS) || state.is(Blocks.TALL_GRASS) ||
                state.is(Blocks.SEAGRASS) || state.is(Blocks.TALL_SEAGRASS) ||
                state.is(Blocks.SNOW) || state.is(Blocks.SNOW_BLOCK) ||
                state.is(Blocks.ICE) || state.is(Blocks.FROSTED_ICE) ||
                state.is(Blocks.BLUE_ICE) || state.is(Blocks.PACKED_ICE) ||
                state.is(BlockTags.FLOWERS)) {
            level.removeBlock(pos, false);
        } else if (state.is(Blocks.GLASS) || state.is(Blocks.GLASS_PANE)) {
            if (random.nextFloat() < 0.6F) {
                level.removeBlock(pos, false);
            }
        }
    }

    private static void applyZone5Effects(
            ServerLevel level,
            BlockPos pos,
            BlockState state,
            Block wasteLogBlock,
            Block wastePlanksBlock,
            RandomSource random) {

        if (state.is(BlockTags.LEAVES)) {
            if (random.nextFloat() < 0.7F) {
                level.removeBlock(pos, false);
            } else if (random.nextFloat() < 0.3F) {
                level.setBlock(pos, Blocks.FIRE.defaultBlockState(), 3);
            }
            return;
        }

        if (state.is(BlockTags.LOGS)) {
            level.setBlock(pos, wasteLogBlock.defaultBlockState(), 3);
            return;
        }

        if (state.is(BlockTags.PLANKS)) {
            level.setBlock(pos, wastePlanksBlock.defaultBlockState(), 3);
            return;
        }

        if (state.is(BlockTags.WOODEN_STAIRS) || state.is(BlockTags.WOODEN_SLABS) ||
                state.is(BlockTags.WOODEN_TRAPDOORS) || state.is(Blocks.TORCH) ||
                state.is(BlockTags.WOOL_CARPETS) || state.is(BlockTags.WOOL) ||
                state.is(BlockTags.WOODEN_FENCES) || state.is(Blocks.PUMPKIN) ||
                state.is(BlockTags.WOODEN_DOORS)) {
            level.removeBlock(pos, false);
        }
    }

    private static void applyZone6Effects(
            ServerLevel level,
            BlockPos pos,
            BlockState state,
            RandomSource random) {

        if (state.is(BlockTags.LEAVES)) {
            if (random.nextFloat() < 0.5F) {
                level.removeBlock(pos, false);
            } else if (random.nextFloat() < 0.1F) {
                level.setBlock(pos, Blocks.FIRE.defaultBlockState(), 3);
            }
        }
    }

    private static void tryApplyNuclearFallout(ServerLevel level, BlockPos pos, BlockState blockBelowState, RandomSource random) {
        if (random.nextFloat() < 0.1F) {
            if (!level.getBlockState(pos).isAir()) return;
            if (!Block.isFaceFull(blockBelowState.getCollisionShape(level, pos.below()), net.minecraft.core.Direction.UP)) {
                return;
            }
            if (!blockBelowState.getFluidState().isEmpty() ||
                    blockBelowState.is(Blocks.FIRE) ||
                    blockBelowState.is(Blocks.SOUL_FIRE) ||
                    blockBelowState.is(Blocks.MAGMA_BLOCK)) {
                return;
            }
            // 原版无核辐射块，此处可自行定义，或使用其他方块（如岩浆）
            // level.setBlock(pos, Blocks.LAVA.defaultBlockState(), 3);
        }
    }

    private static void applyExplosionKnockback(LivingEntity entity, BlockPos centerPos, double radius) {
        double dx = entity.getX() - centerPos.getX();
        double dz = entity.getZ() - centerPos.getZ();
        double distance = Math.sqrt(dx * dx + dz * dz);

        if (distance < 0.1) distance = 0.1;

        double dirX = dx / distance;
        double dirZ = dz / distance;

        double knockbackStrength = Math.max(0, 1.0 - (distance / radius)) * 1.5;

        entity.push(dirX * knockbackStrength, 0.5, dirZ * knockbackStrength);
    }

    private static boolean isEntityExposed(ServerLevel level, Vec3 startPos, LivingEntity entity) {
        Vec3 currentPos = startPos;
        Vec3 targetPos = entity.getEyePosition();
        Vec3 direction = targetPos.subtract(startPos).normalize();
        double maxDistSq = startPos.distanceToSqr(targetPos);

        int safetyLoopCount = 0;

        while (safetyLoopCount++ < 50) {
            if (currentPos.distanceToSqr(startPos) >= maxDistSq) {
                return true;
            }

            ClipContext context = new ClipContext(
                    currentPos,
                    targetPos,
                    ClipContext.Block.COLLIDER,
                    ClipContext.Fluid.NONE,
                    entity
            );

            BlockHitResult result = level.clip(context);

            if (result.getType() == HitResult.Type.MISS) {
                return true;
            }

            BlockPos hitPos = result.getBlockPos();
            BlockState hitState = level.getBlockState(hitPos);

            float defense = BlockExplosionDefense.getBlockDefenseValue(level, hitPos, hitState);

            if (defense >= 50.0F) {
                return false;
            }

            currentPos = result.getLocation().add(direction.scale(0.1));
        }

        return true;
    }

    private static void applyDamageToEntities(
            ServerLevel level,
            BlockPos centerPos,
            double zone3Radius,
            double zone4Radius,
            RandomSource random) {

        MinecraftServer server = level.getServer();
        if (server == null) return;

        double zone6Radius = zone4Radius + 24.0;
        int maxRadiusBlocks = (int) Math.ceil(zone6Radius + 5.0);

        net.minecraft.world.level.ChunkPos minChunk = new net.minecraft.world.level.ChunkPos(centerPos.offset(-maxRadiusBlocks, 0, -maxRadiusBlocks));
        net.minecraft.world.level.ChunkPos maxChunk = new net.minecraft.world.level.ChunkPos(centerPos.offset(maxRadiusBlocks, 0, maxRadiusBlocks));

        List<net.minecraft.world.level.ChunkPos> chunksToProcess = new ArrayList<>();
        for (int x = minChunk.x; x <= maxChunk.x; x++) {
            for (int z = minChunk.z; z <= maxChunk.z; z++) {
                chunksToProcess.add(new net.minecraft.world.level.ChunkPos(x, z));
            }
        }

        processDamageChunkBatch(level, centerPos, chunksToProcess, 0, zone3Radius, zone4Radius, random, server);
    }

    private static void processDamageChunkBatch(ServerLevel level, BlockPos centerPos, List<net.minecraft.world.level.ChunkPos> allChunks, int startIndex, double z3, double z4, RandomSource random, MinecraftServer server) {
        int BATCH_SIZE = 16;
        int endIndex = Math.min(startIndex + BATCH_SIZE, allChunks.size());

        double z5 = z4 + 12.0;
        double z6 = z4 + 24.0;
        double r3Sq = z3 * z3; double r4Sq = z4 * z4; double r5Sq = z5 * z5; double r6Sq = z6 * z6;

        Vec3 explosionCenter = new Vec3(centerPos.getX() + 0.5, centerPos.getY() + 0.5, centerPos.getZ() + 0.5);

        for (int i = startIndex; i < endIndex; i++) {
            net.minecraft.world.level.ChunkPos cp = allChunks.get(i);
            if (!level.hasChunk(cp.x, cp.z)) continue;

            net.minecraft.world.level.chunk.LevelChunk chunk = level.getChunk(cp.x, cp.z);
            AABB box = new AABB(cp.getMinBlockX(), level.getMinBuildHeight(), cp.getMinBlockZ(), cp.getMaxBlockX()+1, level.getMaxBuildHeight(), cp.getMaxBlockZ()+1);

            List<LivingEntity> entities = level.getEntitiesOfClass(LivingEntity.class, box);

            for (LivingEntity ent : entities) {
                double distSq = ent.distanceToSqr(explosionCenter);
                if (distSq > r6Sq) continue;
                if (!isEntityExposed(level, explosionCenter, ent)) continue;

                if (distSq <= r3Sq) {
                    ent.hurt(level.damageSources().generic(), ZONE_3_DAMAGE);
                    ent.setSecondsOnFire((int) FIRE_DURATION / 20);
                    applyExplosionKnockback(ent, centerPos, z3);
                } else if (distSq <= r4Sq) {
                    ent.hurt(level.damageSources().generic(), ZONE_4_DAMAGE);
                    ent.setSecondsOnFire((int) FIRE_DURATION / 20);
                    applyExplosionKnockback(ent, centerPos, z4);
                } else if (distSq <= r5Sq) {
                    ent.hurt(level.damageSources().generic(), 500.0F);
                    ent.setSecondsOnFire(10);
                    applyExplosionKnockback(ent, centerPos, z5);
                } else {
                    ent.hurt(level.damageSources().generic(), 100.0F);
                    applyExplosionKnockback(ent, centerPos, z6);
                }
            }
        }

        if (endIndex < allChunks.size()) {
            server.tell(new TickTask(server.getTickCount() + 1, () ->
                    processDamageChunkBatch(level, centerPos, allChunks, endIndex, z3, z4, random, server)
            ));
        }
    }

    private static void cleanupItems(ServerLevel level, BlockPos center, double radius) {
        AABB area = new AABB(center).inflate(radius, 100, radius);
        List<ItemEntity> items = level.getEntitiesOfClass(ItemEntity.class, area);
        for (ItemEntity item : items) {
            item.discard();
        }
    }

    private static boolean isDebugScreenEnabled() {
        // 可根据需要实现调试检测，此处返回 false 避免依赖客户端
        return false;
    }
}