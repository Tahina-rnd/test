import {
  Injectable,
  NotFoundException,
  ConflictException,
} from '@nestjs/common';
import { InjectRepository } from '@nestjs/typeorm';
import { Repository, Between } from 'typeorm';
import { Attendance } from './entities/attendance.entity';
import { Student } from '../students/entities/student.entity';
import { CreateAttendanceDto } from './dto/create-attendance.dto';
import { UpdateAttendanceDto } from './dto/update-attendance.dto';
import { AttendanceResponseDto } from './dto/attendance-response.dto';
import { AttendanceStatus } from './entities/attendance.entity';

@Injectable()
export class AttendanceService {
  constructor(
    @InjectRepository(Attendance)
    private attendanceRepository: Repository<Attendance>,
    @InjectRepository(Student)
    private studentRepository: Repository<Student>,
  ) {}

  /**
   * Create a new attendance record.
   *
   * @param createAttendanceDto - The attendance data to create.
   *
   * @returns A promise that resolves to the created attendance record.
   *
   * @throws NotFoundException - If the student is not found.
   * @throws ConflictException - If attendance for this session already exists.
   */
  async create(
    createAttendanceDto: CreateAttendanceDto,
  ): Promise<AttendanceResponseDto> {
    // Check if student exists
    const student = await this.studentRepository.findOne({
      where: { id: createAttendanceDto.studentId },
    });

    if (!student) {
      throw new NotFoundException(
        `Student with id ${createAttendanceDto.studentId} not found`,
      );
    }

    // Check if attendance record already exists for this session
    const existingAttendance = await this.attendanceRepository.findOne({
      where: {
        studentId: createAttendanceDto.studentId,
        sessionDate: new Date(createAttendanceDto.sessionDate),
        subjectCode: createAttendanceDto.subjectCode,
        sessionType: createAttendanceDto.sessionType,
      },
    });

    if (existingAttendance) {
      throw new ConflictException(
        'Attendance record already exists for this session',
      );
    }

    const attendance = this.attendanceRepository.create({
      ...createAttendanceDto,
      sessionDate: new Date(createAttendanceDto.sessionDate),
      checkInTime: createAttendanceDto.checkInTime
        ? new Date(createAttendanceDto.checkInTime)
        : undefined,
      checkOutTime: createAttendanceDto.checkOutTime
        ? new Date(createAttendanceDto.checkOutTime)
        : undefined,
    });

    const savedAttendance = await this.attendanceRepository.save(attendance);
    return this.mapToResponseDto(savedAttendance);
  }

  /**
   * Find all attendance records.
   *
   * @returns A promise that resolves to an array of attendance records.
   */
  async findAll(): Promise<AttendanceResponseDto[]> {
    const attendance = await this.attendanceRepository.find({
      relations: ['student'],
      order: { sessionDate: 'DESC' },
    });
    return attendance.map((record) => this.mapToResponseDto(record));
  }

  /**
   * Find an attendance record by id.
   *
   * @param id - The id of the attendance record to find.
   *
   * @returns A promise that resolves to the attendance record.
   *
   * @throws NotFoundException - If the attendance record is not found.
   */
  async findOne(id: string): Promise<AttendanceResponseDto> {
    const attendance = await this.attendanceRepository.findOne({
      where: { id },
      relations: ['student'],
    });

    if (!attendance) {
      throw new NotFoundException(`Attendance record with id ${id} not found`);
    }

    return this.mapToResponseDto(attendance);
  }

  /**
   * Find attendance records by student id.
   *
   * @param studentId - The student id to find attendance records for.
   *
   * @returns A promise that resolves to an array of attendance records.
   */
  async findByStudentId(studentId: string): Promise<AttendanceResponseDto[]> {
    const attendance = await this.attendanceRepository.find({
      where: { studentId },
      relations: ['student'],
      order: { sessionDate: 'DESC' },
    });

    return attendance.map((record) => this.mapToResponseDto(record));
  }

  /**
   * Find attendance records by subject code.
   *
   * @param subjectCode - The subject code to find attendance records for.
   *
   * @returns A promise that resolves to an array of attendance records.
   */
  async findBySubject(subjectCode: string): Promise<AttendanceResponseDto[]> {
    const attendance = await this.attendanceRepository.find({
      where: { subjectCode },
      relations: ['student'],
      order: { sessionDate: 'DESC' },
    });

    return attendance.map((record) => this.mapToResponseDto(record));
  }

  /**
   * Find attendance records by date range.
   *
   * @param startDate - The start date.
   * @param endDate - The end date.
   *
   * @returns A promise that resolves to an array of attendance records.
   */
  async findByDateRange(
    startDate: Date,
    endDate: Date,
  ): Promise<AttendanceResponseDto[]> {
    const attendance = await this.attendanceRepository.find({
      where: {
        sessionDate: Between(startDate, endDate),
      },
      relations: ['student'],
      order: { sessionDate: 'DESC' },
    });

    return attendance.map((record) => this.mapToResponseDto(record));
  }

  /**
   * Calculate attendance percentage for a student.
   *
   * @param studentId - The student id to calculate attendance percentage for.
   *
   * @returns A promise that resolves to the attendance percentage.
   */
  async calculateAttendancePercentage(studentId: string): Promise<number> {
    const attendance = await this.attendanceRepository.find({
      where: { studentId },
    });

    if (attendance.length === 0) {
      return 0;
    }

    const presentCount = attendance.filter(
      (record) =>
        record.status === AttendanceStatus.PRESENT ||
        record.status === AttendanceStatus.LATE,
    ).length;

    return (presentCount / attendance.length) * 100;
  }

  /**
   * Get attendance statistics for a subject.
   *
   * @param subjectCode - The subject code to get statistics for.
   *
   * @returns A promise that resolves to attendance statistics.
   */
  async getSubjectAttendanceStats(subjectCode: string): Promise<{
    totalSessions: number;
    presentCount: number;
    absentCount: number;
    lateCount: number;
    excusedCount: number;
    attendanceRate: number;
  }> {
    const attendance = await this.attendanceRepository.find({
      where: { subjectCode },
    });

    const stats = {
      totalSessions: attendance.length,
      presentCount: attendance.filter(
        (a) => a.status === AttendanceStatus.PRESENT,
      ).length,
      absentCount: attendance.filter(
        (a) => a.status === AttendanceStatus.ABSENT,
      ).length,
      lateCount: attendance.filter((a) => a.status === AttendanceStatus.LATE)
        .length,
      excusedCount: attendance.filter(
        (a) => a.status === AttendanceStatus.EXCUSED,
      ).length,
      attendanceRate: 0,
    };

    stats.attendanceRate =
      stats.totalSessions > 0
        ? ((stats.presentCount + stats.lateCount) / stats.totalSessions) * 100
        : 0;

    return stats;
  }

  /**
   * Update an attendance record.
   *
   * @param id - The id of the attendance record to update.
   * @param updateAttendanceDto - The attendance data to update.
   *
   * @returns A promise that resolves to the updated attendance record.
   *
   * @throws NotFoundException - If the attendance record is not found.
   */
  async update(
    id: string,
    updateAttendanceDto: UpdateAttendanceDto,
  ): Promise<AttendanceResponseDto> {
    const attendance = await this.attendanceRepository.findOne({
      where: { id },
    });

    if (!attendance) {
      throw new NotFoundException(`Attendance record with id ${id} not found`);
    }

    const updatedAttendance = await this.attendanceRepository.save({
      ...attendance,
      ...updateAttendanceDto,
      sessionDate:
        updateAttendanceDto.sessionDate !== undefined
          ? new Date(updateAttendanceDto.sessionDate)
          : attendance.sessionDate,
      checkInTime:
        updateAttendanceDto.checkInTime !== undefined
          ? new Date(updateAttendanceDto.checkInTime)
          : attendance.checkInTime,
      checkOutTime:
        updateAttendanceDto.checkOutTime !== undefined
          ? new Date(updateAttendanceDto.checkOutTime)
          : attendance.checkOutTime,
    });

    return this.mapToResponseDto(updatedAttendance);
  }

  /**
   * Remove an attendance record (soft delete).
   *
   * @param id - The id of the attendance record to remove.
   *
   * @throws NotFoundException - If the attendance record is not found.
   */
  async remove(id: string): Promise<void> {
    const attendance = await this.attendanceRepository.findOne({
      where: { id },
    });

    if (!attendance) {
      throw new NotFoundException(`Attendance record with id ${id} not found`);
    }

    await this.attendanceRepository.softDelete(id);
  }

  /**
   * Map an Attendance entity to AttendanceResponseDto.
   *
   * @param attendance - The attendance entity to map.
   *
   * @returns The mapped AttendanceResponseDto.
   */
  private mapToResponseDto(attendance: Attendance): AttendanceResponseDto {
    return {
      id: attendance.id,
      studentId: attendance.studentId,
      subjectCode: attendance.subjectCode,
      subjectName: attendance.subjectName,
      sessionDate: attendance.sessionDate,
      sessionType: attendance.sessionType,
      status: attendance.status,
      checkInTime: attendance.checkInTime,
      checkOutTime: attendance.checkOutTime,
      durationMinutes: attendance.durationMinutes,
      notes: attendance.notes,
      markedBy: attendance.markedBy,
      academicYear: attendance.academicYear,
      semester: attendance.semester,
      createdAt: attendance.createdAt,
      updatedAt: attendance.updatedAt,
    };
  }
}
